// Menu for set the datetime in timeLoop

#define SET_DT_CURSOR_POS_CNT 6

byte cursor_index_time_set = 1;
const byte cursPos_time_set[SET_DT_CURSOR_POS_CNT] = {0, 1, 4, 9, 12, 15};

void init_setDateTime() {
    lcd.cursor();
    lcd.blink();
    byte cursor_index_time_set = 1;
}

void display_setDateTime() {

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set time  " + tmh.getDayOfWeekName(0) + "  " + String(tmh.getSec())); 
    lcd.setCursor(0,1);
    /// pos        1  4    9  c  f
    /// show     "dd.mm.yyyy hh:mm" the :ss is cut of
    lcd.print(tmh.getDayMonYear()+ " " + tmh.getHrsMinSec());
    lcd.setCursor(cursPos_time_set[cursor_index_time_set], 1);
}

void handleKeystroke_setDateTime() {
    switch (kbdValue) {
    case KEY_ENTER:   // enter (key 0 in set date time mode
        // finish
        displayMode = 0;
        kbdValue = 255; 
        lcd.noCursor();
        lcd.noBlink();
        tmh.resetSecToZero();
        storeEEprom();
        break;
    case KEY_RIGHT: // right (key 1 in set date time mode
        cursor_index_time_set++;
        if (cursor_index_time_set >= SET_DT_CURSOR_POS_CNT) { cursor_index_time_set = 1; }
        break;
    case KEY_LEFT: // left (key 2 in set date time mode
        cursor_index_time_set--;
        if (cursor_index_time_set < 1) { cursor_index_time_set = SET_DT_CURSOR_POS_CNT - 1; }
        break;
    case KEY_UP: // up 
        switch (cursor_index_time_set) {
            case 1: //day
                tmh.incrementDayCounter(1);
                break;
            case 2: //day
                tmh.incrementMonth();
                break;
            case 3: //day
                tmh.incrementYear();
                break;
            case 4: //day
                tmh.incrementSecondsCounter(3600);
                break;
            case 5: //day
                tmh.incrementSecondsCounter(60);
                break;
        }
        break;
    case KEY_DOWN: // down 
        switch (cursor_index_time_set) {
            case 1: //day
                tmh.incrementDayCounter(-1);
                break;
            case 2: //day
                tmh.decrementMonth();
                break;
            case 3: //day
                tmh.decrementYear();
                break;
            case 4: //day
                tmh.incrementSecondsCounter(-3600);
                break;
            case 5: //day
                tmh.incrementSecondsCounter(-60);
                break;
        }
        break;
                    
    lcd.setCursor(0,1);
                
    default:
      break;
    
    }
}
  
