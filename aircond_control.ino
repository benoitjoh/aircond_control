/*  Controller for MACS aircondinioning system
    
    * includes 
    * timer 
    * 4 Temperatur sensors
    * pcc for fan motor (235V)
    * relais switch for compressor
    * solid state relais for cooling water pum
    * 12V switch for cooler fans (5 * 1.5W fan)
    * 
    */

#include "config.h"

#define VERSION "v1.0/" __DATE__


// alarm for second temp sensor
// alarm limit defined in storage.cfg[CONF_FRIDGE_ALARM]
boolean fridge_alarm = false;
boolean fridge_quit = false;

unsigned long power;
unsigned long nowMillis;
unsigned long lastIntervalStart;
unsigned long lastTimeDeltaMillis;

int temp = 0;
int temp_2 = 0;


byte displayMode = 0; 
long resetDisplayModeSeconds = 0;
byte actualMonth = 0;

char statusSign; 
byte backlight = 1;


// --- liquid crystal display driver from LiquidCrystal.h or LiquidCrystal_SR2W.h ---------------------------------------------
#ifdef USE_STANDARD_LCD_LIBRARY
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

// ---- string helper   ---------------------------------------------------------------  
String leftFill(String a, byte wantedLen, String fillLetter) {
    // fills a string with letters from left side that the resulting length is reached
    while (a.length() < wantedLen) {
        a = fillLetter + a;
        }
        
    for (byte i=0; i<wantedLen; i++) {
        if (a[i] == '-') {
            a[i] = 4;
            }
        }
    return a;
    }

byte divideAndRoundBy(unsigned long value, int dividend) {
    // divide by dividend [10, 100, 1000] and round 
    byte result = byte(value / dividend);
    if ( value % dividend  > ((dividend / 2) - 1) ) {
        result +=1;
        }
}

// EEPROM ------------------------------------------------------------------
#include <EEPROM.h>

#define EE_OFFSET 0  // start adress for the EEPROM data area 
#define CONF_ADDR_COUNT   8  // how many values do we have in the conf array? 

// --- Variables that will be stored in eeprom persistent actually 100byte long --- 
struct {
long secondsCounter = 79200;
long dayCounter = 1555;
int cfg[CONF_ADDR_COUNT] = {450, -10, 9, 0, 0, 0, 0, 0, };
} storage;

// *** config paramters adjustable in configMenu ************************************** //
#define CONF_TIMECORR_MS_PER_HOUR 0
#define CONF_FRIDGE_ALARM 1
#define CONF_V2 2
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

    lcd.print(F(" # MACS 7000.jb #")); 
    lcd.setCursor(0,1);
    lcd.print(VERSION);
    delay(2000);

    loadEEprom(EE_OFFSET );
    actualMonth = tmh.getMonth(0);
    setBacklight(1);

    pinMode(13, OUTPUT);
    
    // serial device for debugging purpose!
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial Pin to connect. otherwise reset if serial console is started :-/
        }
        
    statusSign = 32;

}


// Mainloop ---------------------------------------------------------------------------

void loop() {
    nowMillis = millis();
    // ---- do something ---------

    // update the timeloop handler. state: 0: nothing; 1: 100ms passed; 2: 1s passed; 3: midnight
    byte timeState = tmh.actualize(); 

    if ( timeState >= 1 ) {
        // --- 100ms has passed ---- 

        if ( fridge_alarm ) {
            backlight = !backlight;
            }
        else {
            backlight = 1;
            }

        setBacklight(backlight);

        if ( timeState >= 2 ) {
            // --- 1 sec has passed ---- 

            PORTB |=  B00100000; //set pin13 to HIGH, give led a quick blink each second     

            delayMicroseconds(50);
            PORTB &= ~B00100000; //set pin13 to LOW 
            
            temp = read_pt1000(analogRead(ANALOG_TEMP_1_PIN));
            temp_2 = read_pt1000(analogRead(ANALOG_TEMP_2_PIN));

            
            if ( tmh.getSecondsCounter() % 3600 == 1) {
                // ----- new hour started... so reset the actual counter 
                ; 
                }


            if ( timeState >= 3 ) {
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
                lcd.setCursor(0,0);
                lcd.print(leftFill(String(111), 4, " ") + "W ");
                lcd.write(statusSign);
                lcd.print(" " + leftFill(String(111, 2), 5, " ") + "kWh");
                lcd.setCursor(0,1);
                lcd.print(leftFill(String(temp), 2, " ") + "\xdf" + "" +
                          leftFill(String(temp_2), 3, " ") + "\xdf" + " " +
                          tmh.getHrsMinSec());
                break;

            case 2:
                // -- display overall kWh
                lcd.clear();
                lcd.print("Ges.: " + leftFill(String(111, 0), 6, " ") + " kWh");
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
                display_setDateTime();
                break;
                


            }
        
        if (displayMode < 30 and displayMode > 1 and
            tmh.getSecondsCounter() > resetDisplayModeSeconds ) {
            // fall back to standard display after 5 seconds exept display mode over 30 
            // these displays should be manually reset
            displayMode = 0;
            }


        } // --- 100ms

    else {
        delay(1); //minimum loop time if nothing else happened...       
        }

    // ---- deal with an eventually pressed key -----------------------
    kbdValue = kbd.read();

        if (kbdValue != 255) { //key is pressed
            // reset fridge alarm
            fridge_alarm = false; 
            fridge_quit = true; 

            if (displayMode == 99) {
                // if its 99 (set date time) then keys for set date time is active
                // keys 0, 1, 2, 3, 4 as 20, 21, 22, 23 ,24 
                kbdValue += 20;
                }

            if (displayMode == 98) { // set config menu
                kbdValue += 30;
                }
            Serial.println("kbd:" + String(kbdValue) + " ad:" + String(kbd.getLastAdValue()));
            
            switch (kbdValue)
                {
                case KEY_ENTER:
                    storeEEprom();
                    break;

                case KEY_RIGHT:
                    displayMode += 2; // flip through the history pages
                    if ( displayMode > 20 ) {
                        displayMode = 0;
                    }
                    resetDisplayModeSeconds = tmh.getSecondsCounter() + 5;
                    break;

                case KEY_LEFT:
                    displayMode = 30; //debug
                    resetDisplayModeSeconds = tmh.getSecondsCounter() + 5;
                    break;

                case KEY_UP:
                    // tmh.incrementSecondsCounter(60);
                    break;

                case KEY_DOWN:
                    // tmh.incrementSecondsCounter(-60);
                    break;

                case KEY_ENTER_LONG:
                    displayMode = 99; // start setDateTime 
                    break;

                case KEY_DOWN_LONG:
                    displayMode = 98; // start setConfig 
                    break;

                case 20 ... 24: 
                    handleKeystroke_setDateTime();
                    break;

                default:
                    break;
                }

            handleKeystroke_setConfigMenu();
        }
}
