// Controller for MACS aircondinioning system

/*      
    * includes 
    * timer 
    * 4 Temperatur sensors
    * pcc for fan motor (235V)
    * relais switch for compressor
    * solid state relais for cooling water pum
    * 12V switch for cooler fans (5 * 1.5W fan)
    * 

    Display
    ----------------
    [20-25] kü   VKP   [temp range], kü|hz, Ventilator, Kompressor, Pumpe 
    20.2° AUTO 12:23   akt. temp, AUTO/EIN/AUS, Time
    ----------------
    
    */

#include "config.h"

#define VERSION "v1.0/" __DATE__

// ---- special characters   ---------------------------------------------------------------  
#define CHAR_HOOK 1
#define CHAR_ARROW_UP 2
#define CHAR_ARROW_DOWN 3
#define CHAR_DIVISE 4
#define CHAR_P_DOTTED 5
#define CHAR_V_DOTTED 6
#define CHAR_SCORE 7


// alarm for second temp sensor
// alarm limit defined in storage.cfg[CONF_TEMP_ALARM]
boolean alarm_on = false;

unsigned long start_millis;

float temp = 22.2;
int temp_1 = 0;
int temp_2 = 0;

// State variables
byte run_mode = 0; // Manual setting.  0: off  1:  on  2: clock controlled 3: only fan is on
byte run_mode_old = 0;
byte run_mode_change_delay = 0;

byte engine_state = 0; // 0 system  off, 1 system is  temp controlled

byte timer_active = 0;

byte function_state = 0; // 0: off, 1: cooling 2: heat  ;

byte fan_state = 0; // off/on/ wait for off
byte compressor_state = 0; // off/on
byte valve_state = 0; // off/on

byte pump_state = 0; // 0: off / 1: on / 2: wait for off
byte pump_wait= 0; // count down for switch of pump

String run_mode_name[4] {" AUS", " EIN", "AUTO", "VENT"};
char function_state_name[3][2] {{CHAR_SCORE, CHAR_SCORE}, {'k', '\xf5'}, {'h', 'z'}};

char fan_state_name[3] {CHAR_SCORE, 'v', CHAR_V_DOTTED};
byte compressor_state_name[2] {CHAR_SCORE, 'k'};
byte pump_state_name[3] {CHAR_SCORE, 'p', CHAR_P_DOTTED};



byte display_page = 0; 
long resetDisplayModeSeconds = 0;
long reset_backlight_seconds = 0;

byte backlight = 1;


// --- liquid crystal display driver from LiquidCrystal.h or LiquidCrystal_SR2W.h ---------------------------------------------
#if USE_STANDARD_LCD_LIBRARY == 1
    // using the standard LCD Library
    #include <LiquidCrystal.h>
    //            lcd(rs, e, d4, d5, d6, d7);   backlight: pin D6
    LiquidCrystal lcd(LCD_RS,  LCD_E,   LCD_D4, LCD_D5, LCD_D6, LCD_D7);

    void setBacklight(byte state) {
        digitalWrite(LCD_BACKLIGHT, state);
        }
#else
    // using the latchregister
    #include <LiquidCrystal_SR2W.h>
    LiquidCrystal_SR2W lcd(LCD_DATA, LCD_CLOCK, POSITIVE);

    void setBacklight(byte state) {
        lcd.setBacklight(state);
        }
#endif // USE_STANDARD_LCD_LIBRARY


// --- keyboard driver from AnalogKbd.cpp ---------------------------------------------
#include<AnalogKbd.h>

#define NO_KBD_PRESSED 255
AnalogKbd kbd(PIN_ANALOG_KBD, KBD_NR_OF_KEYS);
byte kbdValue = NO_KBD_PRESSED; //the value that is read from keyboard 255 is neutral value


// --- timer helpers from TimeLoop.cpp ---------------------------------------------
#include<TimeLoop.h>
TimeLoop tmh(0);

// --- library for MCP9808 temperature sensor 
#if USE_MCP9808_SENSOR == 1
    #include <Wire.h>
    #include "Adafruit_MCP9808.h"

    // Create the MCP9808 temperature sensor object
    Adafruit_MCP9808 mcp_tempsensor = Adafruit_MCP9808();
#endif

// --- library for phase cut controller ----------------------------------------------
#include <PhaseCutCtrl.h>
// Note: PhaseCutCtrl is preinstantiated as PCCtrl. Initialization later in setup()


// EEPROM ------------------------------------------------------------------
#include <EEPROM.h>

#define EE_OFFSET 0  // start adress for the EEPROM data area 
#define CONF_ADDR_COUNT   8  // how many values do we have in the conf array? 

// --- Variables that will be stored in eeprom persistent actually 100byte long --- 
struct {
long secondsCounter = 79200;
long dayCounter = 1555;
byte temp_min = 20;
byte temp_max = 25;
byte switch_times[4][3] = { 
    {6, 20, 1}, {10, 30, 0}, {0, 0, 1}, {0, 0, 0} }; // {hour, min, up/down 1|0 }
int cfg[CONF_ADDR_COUNT] = {450, -10, 9, 0, 0, 0, 0, 0, };
} storage;

// *** config paramters adjustable in configMenu ************************************** //
#define CONF_TIMECORR_MS_PER_HOUR 0
#define CONF_TEMP_ALARM 1
#define CONF_FAN_SPEED 2
#define CONF_PUMP_WAIT 3
#define CONF_HYSTERESIS 4
#define CONF_DISP_DELAY 5
#define CONF_V6 6
#define CONF_V7 7


void loadEEprom(bool reset, int offset) {
    lcd.setCursor(0,1);
    if (!reset) {
        EEPROM.get(offset, storage);
        lcd.print(F("data loaded.     "));
    }
    else {
        // load values from default above.
        lcd.print(F("EEPROM reset.     "));
        delay(500);
        // comment in to reset the day of year values 
        /*for (int i=0; i<367; i++) {
            updateDayOfYearTotal(i, 0);
        }*/
    }
    
    // pass the values to TimeLoop
    tmh.setSecondsCounter(storage.secondsCounter);
    tmh.setDayCounter(storage.dayCounter);
    tmh.setMsPerHourCorrection(storage.cfg[CONF_TIMECORR_MS_PER_HOUR]);
    delay(800);
    }


void storeEEprom() {
    // apply configuration changes, and store all to eprom 
    
    // fetch values from TimeLoop
    storage.secondsCounter = tmh.getSecondsCounter();
    storage.dayCounter = tmh.getDayCounter();
    tmh.setMsPerHourCorrection(storage.cfg[CONF_TIMECORR_MS_PER_HOUR]);
    
    EEPROM.put(EE_OFFSET, storage);
    lcd.setCursor(0,1);
    lcd.print(F("data saved. "));
    delay(500);
    }


// Initialization ---------------------------------------------------------------------------

void setup() {
    display_page = 4; // 0 means actual day.
    resetDisplayModeSeconds = 0;

    init_lcd(); // init and add userdefined chars

    lcd.noCursor();

    lcd.print(F("#  AC control  #")); 
    lcd.setCursor(0,1);
    lcd.print(VERSION);
    delay(1000);

    // if no key is pressed on start, read values from eeprom, else reset
    bool doReset = false;
    if (analogRead(PIN_ANALOG_KBD) > 100) {
        lcd.setCursor(0,0);
        lcd.print(F("ovewrite EEPROM?"));
        lcd.setCursor(0,1);
        lcd.print(F("  yes --> Enter "));
        byte kbdResult = kbd.wait_till_read();
        doReset = bool(kbdResult == 0);
        }

    loadEEprom(doReset, EE_OFFSET );
    setBacklight(1);

    pinMode(13, OUTPUT);
    pinMode(PIN_COMPRESSOR, OUTPUT);
    pinMode(PIN_PUMP, OUTPUT);
    pinMode(PIN_VALVE, OUTPUT);
    pinMode(PIN_COOLERFAN, OUTPUT);
    
    // serial device for debugging purpose!
    Serial.begin(9600);
    while (!Serial) {
        ; // wait for serial Pin to connect. otherwise reset if serial console is started :-/
        }

    #if USE_MCP9808_SENSOR == 1
        if (!mcp_tempsensor.begin(0x18)) {
            lcd.setCursor(0,1);
            lcd.print(F("No MCP9808 conn!"));
            while (1);
        }
        mcp_tempsensor.setResolution(2); // sets the resolution to 0.125 °C
    #endif

    // Phase cut Ctrl module (for regulation of the 235V fan motor)
    PCCtrl.initialize(PIN_IN_ACZERO_SIGNAL, PIN_PCC_OUT_A);

}


// Mainloop ---------------------------------------------------------------------------

void loop() {
    start_millis = millis();
    // ---- do something ---------
    
    // checkAcDisconnected();

    // update the timeloop handler. state: 0: nothing; 1: 100ms passed; 2: 1s passed; 3: midnight
    byte time_state = tmh.actualize(); 

    if ( time_state >= 1 ) {
        // --- 100ms has passed ---- 
        toggle_backlight(alarm_on);

                    
        // check manual change of run_mode 
        if ( run_mode != run_mode_old ) {
            run_mode_change_delay++;
            if ( run_mode_change_delay > 12 ) {
                // ----- handle change here -----------
                run_mode_old = run_mode;
                run_mode_change_delay = 0;

                flash(20); delay(50); flash(20);
                
                switch (run_mode) {
                    case 0:
                        engine_state = 0;
                        request_stop_compressor();
                        timer_active = 0;
                        break;
                    case 1:
                        engine_state = 1;
                        timer_active = 0;
                        break;
                    case 2:
                        timer_active = 1;
                        break;
                    case 3:
                        engine_state = 0;
                        request_stop_compressor();
                        timer_active = 0;
                        fan_state = 1;
                }
            }
        }


        if ( time_state >= 2 ) {
            // --- 1 sec has passed ---- 
            PORTB |=  B00100000; // set led on  
            // Serial.println("ad0:" + String(analogRead(0)) + "  ad1:" + String(analogRead(1)) + "  ad2:" + String(analogRead(2)) + "  ad3:" + String(analogRead(3)));
            // check timer
            if (tmh.getSec() == 1 && timer_active == 1 ) {
                byte minute = tmh.getMinute();
                byte hour = tmh.getHour();
                for (byte i=0; i<4; i++) {
                    // Serial.println("sw.time: " + String(storage.switch_times[i][0]) + ":" + String(storage.switch_times[i][1]));
                    if (storage.switch_times[i][0] == hour &&
                        storage.switch_times[i][1] == minute) {
                            engine_state = storage.switch_times[i][2];
                            // Serial.println(" --> switch to " + String(storage.switch_times[i][2]));
                            if (engine_state == 0) {
                                request_stop_compressor();
                            }
                            flash(20); delay(50); flash(20);

                        }
                    }
            }


            // if pump/fan is waiting for off (pump_state = 2), switch off after delay is passed
            if ( pump_wait > 0 ) { 
                pump_wait--; }
            else {
                if ( pump_state == 2 ) {
                    pump_state = 0;
                }
                if ( fan_state == 2 ) {
                    fan_state = 0;
                }
            }

            delayMicroseconds(50);  // very quick blink
            PORTB &= ~B00100000; //set pin13 to LOW 

            if ( tmh.getSec() % 5 == 0 ) {
                init_lcd();
            }


            
            // read sensors
#if USE_MCP9808_SENSOR == 1
            temp = mcp_tempsensor.readTempC();
#endif
            temp_1 = read_pt1000(analogRead(ANALOG_TEMP_1_PIN));
            temp_2 = read_pt1000(analogRead(ANALOG_TEMP_2_PIN));

            set_function_by_temp_condition();
            set_compressor_by_temp_condition();


            // generate outputs
            if (fan_state == 1) {
                PCCtrl.set_pcc(storage.cfg[CONF_FAN_SPEED]);
            }
            else {
                PCCtrl.set_pcc(0);
            }

            digitalWrite(PIN_COMPRESSOR, compressor_state);
            digitalWrite(PIN_PUMP, pump_state);
            digitalWrite(PIN_VALVE, valve_state);
            digitalWrite(PIN_COOLERFAN, pump_state);

            if ( tmh.getSecondsCounter() % 3600 == 1) {
                // ----- new hour started... 
                ; 
                }

            if ( time_state >= 3 ) {
                // ----  action at midnight:  --------------------------
                storeEEprom();
                }

            
        } // --- 1 sec
        
 
        PORTB &= ~B00100000; //set pin13 to LOW 



        // ---- refresh display ----------------------------------------------------------------------------

        switch ( display_page )
            {
            case 0:
                cursor_off();
                show_page_standard();
                break;

            case 1 ... 2:
                // -- display/edit switch times
                cursor_on();
                display_set_time_switch();
                break; 

            case 3:
                // -- displa/edit temperatur settings
                cursor_on();
                display_set_temp_limit();
                break; 

            case 4:
                // -- debug information values of ad inputs
                show_page_debug_advalues();
                break;
            
            case 98:
                // -- set Time and date
                cursor_on();
                display_setConfigMenu();
                break;

            case 99:
                // -- set Time and date
                cursor_on();
                display_set_date_time();
                break;
            }
        
        if (display_page > 0 and display_page < 30 and
            tmh.getSecondsCounter() > resetDisplayModeSeconds ) {
            // fall back to standard display after a defined time
            // these displays > 30 must be manually reset
            display_page = 0;
            }


        } // --- 100ms


    // ---- deal with an eventually pressed key -----------------------
    kbdValue = kbd.read();

    if (kbdValue != NO_KBD_PRESSED) { // key is pressed
        // Serial.println("kbd:" + String(kbdValue) + " ad:" + String(kbd.getLastAdValue()));
        alarm_on = false; 

        reset_backlight_seconds = tmh.getSecondsCounter() + storage.cfg[CONF_DISP_DELAY];

        switch (display_page) {
            case 1 ... 2:
                handleKeystroke_set_time_switch();
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;
            case 3:
                handle_keystroke_set_temp_limit();
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;
            case 98:
                handleKeystroke_setConfigMenu();
                break;
            case 99:
                handle_keystroke_set_date_time();
                break;
        }

        // handle the unhandled Keys that are left over  (not already set to NO_KBD_PRESSED)   
        switch (kbdValue) {
            case KEY_ENTER:
                storeEEprom();
                display_page = 0;
                cursor_off();
                break;

            case KEY_RIGHT:
                break;

            case KEY_LEFT:
                //display_page = 4; //debug
                //resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;

            case KEY_UP: 
                temp += 0.1; // for test purpose only
                break;

            case KEY_DOWN:
                temp -= 0.1;  // for test purpose only        // reset fridge alarm

                break;

            case KEY_MID:
                run_mode = increment_byte_value(run_mode, 0, 3, 1); // ON OFF AUTO VENT
                run_mode_change_delay = 0;
                break;

            case KEY_LEFTUP:
                display_page = decrement_byte_value(display_page, 0, 4, 1); // flip through the pages
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;

            case KEY_LEFTDOWN:
                display_page = increment_byte_value(display_page, 0, 4, 1); // flip through the pages
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;

            case KEY_ENTER_LONG:
                display_page = 99; // start setDateTime 
                break;

            case KEY_DOWN_LONG:
                display_page = 98; // start setConfig 
                break;

            default:
                break;
        }
    }

    if ( (millis() - start_millis) < 20 ) {
        delay(50);
    }
}