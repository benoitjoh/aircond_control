// Menu for set the datetime in timeLoop

#define SET_DT_CURSOR_POS_CNT 6

byte cursor_index_time_set = 1;
const byte cursPos_time_set[SET_DT_CURSOR_POS_CNT] = {0, 1, 4, 9, 12, 15};


void display_set_date_time() {

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Set time  " + tmh.getDayOfWeekName(0) + "  " + String(tmh.getSec())); 
    lcd.setCursor(0,1);
    /// pos        1  4    9  c  f
    /// show     "dd.mm.yyyy hh:mm" the :ss is cut of
    lcd.print(tmh.getDayMonYear()+ " " + tmh.getHrsMinSec());
    lcd.setCursor(cursPos_time_set[cursor_index_time_set], 1);
}

void handle_keystroke_set_date_time() {
    switch (kbdValue) {
    case KEY_ENTER:   // enter (key 0 in set date time mode
        // finish
        tmh.resetSecToZero();
        break;
    case KEY_RIGHT: // right (key 1 in set date time mode
        kbdValue = NO_KBD_PRESSED;
        cursor_index_time_set++;
        if (cursor_index_time_set >= SET_DT_CURSOR_POS_CNT) { cursor_index_time_set = 1; }
        break;
    case KEY_LEFT: // left (key 2 in set date time mode
        kbdValue = NO_KBD_PRESSED;
        cursor_index_time_set--;
        if (cursor_index_time_set < 1) { cursor_index_time_set = SET_DT_CURSOR_POS_CNT - 1; }
        break;
    case KEY_UP: // up 
        kbdValue = NO_KBD_PRESSED;
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
        kbdValue = NO_KBD_PRESSED;
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
  
