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
    [20-25°] kü  VKP   [temp range], kü|hz, Ventilator, Kompressor, Pumpe 
    20.2° AUTO 12:23   akt. temp, AUTO/EIN/AUS, Time
    ----------------
    
    */

#include "config.h"

#define VERSION "v1.0/" __DATE__


// alarm for second temp sensor
// alarm limit defined in storage.cfg[CONF_TEMP_ALARM]
boolean temp_alarm = false;

unsigned long nowMillis;

int temp = 0;
int temp_1 = 0;
int temp_2 = 0;

byte function_mode = 0; // 0: cooling   1: heating
String function_mode_name[2] {"k\xf5", "hz"};

byte run_mode = 0; // 0: off  1:  on  2: clock controlled 3: only fan is on
String run_mode_name[4] {" AUS", " EIN", "AUTO", "VENT"};

byte run_state = 0; // 0: inactive  1: active. switched by timer or user (by run_mode)


byte displayMode = 0; 
long resetDisplayModeSeconds = 0;

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

AnalogKbd kbd(PIN_ANALOG_KBD, KBD_NR_OF_KEYS);
byte kbdValue = 255; //the value that is read from keyboard 255 is neutral value


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

// ---- special characters   ---------------------------------------------------------------  
#define CHAR_HOOK 1
#define CHAR_ARROW_UP 2
#define CHAR_ARROW_DOWN 3
#define CHAR_DIVISE 4


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
#define CONF_V3 3
#define CONF_V4 4
#define CONF_V5 5
#define CONF_V6 6
#define CONF_V7 7


void loadEEprom(int offset) {
    lcd.setCursor(0,1);
    EEPROM.get(offset, storage);
    lcd.print(F("data loaded.     "));
    
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
    displayMode = 0; // 0 means actual day.
    resetDisplayModeSeconds = 0;

    init_lcd(); // init and add userdefined chars

    lcd.noCursor();

    lcd.print(F("# MACS 7000jb #")); 
    lcd.setCursor(0,1);
    lcd.print(VERSION);
    delay(1000);

    loadEEprom(EE_OFFSET );
    setBacklight(1);

    pinMode(13, OUTPUT);
    
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
    nowMillis = millis();
    // ---- do something ---------
    
    delay(50); // do he loop max 20 times per second
    // checkAcDisconnected();

    // update the timeloop handler. state: 0: nothing; 1: 100ms passed; 2: 1s passed; 3: midnight
    byte time_state = tmh.actualize(); 

    if ( time_state >= 1 ) {
        // --- 100ms has passed ---- 

        if ( temp_alarm ) {
            backlight = !backlight;
            }
        else {
            backlight = 1;
            }

        setBacklight(backlight);

        if ( time_state >= 2 ) {
            // --- 1 sec has passed ---- 

            PORTB |=  B00100000; //set pin13 to HIGH, give led a quick blink each second     
            delayMicroseconds(50);
            PORTB &= ~B00100000; //set pin13 to LOW 
            
            // read sensors
#if USE_MCP9808_SENSOR == 1
            temp = mcp_tempsensor.readTempC();
#endif
            temp_1 = read_pt1000(analogRead(ANALOG_TEMP_1_PIN));
            temp_2 = read_pt1000(analogRead(ANALOG_TEMP_2_PIN));

            PCCtrl.set_pcc(storage.cfg[CONF_FAN_SPEED]);

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

        switch ( displayMode )
            {
            case 0:
                // -- display normal information each second
                //     [20-25°] kü  VKP   [temp range], kü|hz, Ventilator, Kompressor, Pumpe 
                //     20.2° AUTO 12:23   akt. temp, AUTO/EIN/AUS, Time

                lcd.setCursor(0,0);
                lcd.print("[" + leftFill(String(storage.temp_min), 2, " ") + "-" +
                            leftFill(String(storage.temp_max), 2, " ") + "] ");
                lcd.print(function_mode_name[function_mode] + "   ");
                lcd.print("VKP");
                lcd.setCursor(0,1);
                lcd.print(leftFill(String(18.6, 1), 4, " ") + "\xdf" + " ");
                lcd.print(run_mode_name[run_mode] + " ");
                lcd.print(tmh.getHrsMinSec());
                break;

            case 1 ... 2:
                // -- display switch times
                display_set_time_switch(displayMode);
                break; 

            case 3:
                // -- display temperatur settings
                //display_set_temp_limits(displayMode);
                display_set_temp_limit();

                break; 

            case 30:
                // -- debug information values of ad inputs
                lcd.setCursor(0,0);
                lcd.print("  A0  A1  A2  A3");
                lcd.setCursor(0,1);
                lcd.print(leftFill(String(analogRead(0)), 4, " ") +
                    leftFill(String(analogRead(1)), 4, " ") + 
                    leftFill(String(analogRead(2)), 4, " ") + 
                    leftFill(String(analogRead(3)), 4, " ") );
                break;
            
            case 98:
                // -- set Time and date
                display_setConfigMenu();
                break;

            case 99:
                // -- set Time and date
                display_set_date_time();
                break;
            }
        
        if (displayMode > 0 and displayMode < 30 and
            tmh.getSecondsCounter() > resetDisplayModeSeconds ) {
            // fall back to standard display after 5 defined time
            // these disp321lays should be manually reset
            displayMode = 0;
            }


        } // --- 100ms

    else {
        delay(1); //minimum loop time if nothing else happened...       
        }

    // ---- deal with an eventually pressed key -----------------------
    kbdValue = kbd.read();

    if (kbdValue != 255) { //key is pressed
        Serial.println("kbd:" + String(kbdValue) + " ad:" + String(kbd.getLastAdValue()));
        // reset fridge alarm
        temp_alarm = false; 

        switch (displayMode) {
            case 1 ... 2:
                handleKeystroke_set_time_switch(displayMode);
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;
            case 3:
                handle_keystroke_set_temp_limit();
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;
            case 98:
                handleKeystroke_setConfigMenu();
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
               break;
            case 99:
                handle_keystroke_set_date_time();
                resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                break;

            default:
                switch (kbdValue) {
                    case KEY_ENTER:
                        storeEEprom();
                        break;

                    case KEY_RIGHT:
                        break;

                    case KEY_LEFT:
                        displayMode = 30; //debug
                        resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                        break;

                    case KEY_UP:
                        break;

                    case KEY_DOWN:
                         break;

                    case KEY_MID:
                        run_mode = increment_byte_value(run_mode, 0, 3, 1); // ON OFF AUTO VENT
                        break;

                    case KEY_LEFTUP:
                        displayMode = decrement_byte_value(displayMode, 0, 3, 1); // flip through the pages
                        resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                        break;

                    case KEY_LEFTDOWN:
                        displayMode = increment_byte_value(displayMode, 0, 3, 1); // flip through the pages
                        resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
                        break;

                    case KEY_ENTER_LONG:
                        displayMode = 99; // start setDateTime 
                        init_set_date_time();

                        break;

                    case KEY_DOWN_LONG:
                        displayMode = 98; // start setConfig 
                        break;

                    default:
                        break;
                }
                break;
        }
    }
}