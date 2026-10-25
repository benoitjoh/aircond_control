// Menu for set the switch points for the timer



#define SET_TS_CURSOR_POS_CNT 4

byte cursor_index_ts_set = 0;
const byte cursPos_ts[SET_TS_CURSOR_POS_CNT] = {2, 5, 10, 13};

void display_set_time_switch(byte index) {
    // index is 1 or 2. to adress values in array 0, 1 reduce by it by 1
    index--;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Schaltzeit " + String(index) + ":"); 
    lcd.setCursor(0, 1);
    /// pos         2  5   9  c
    /// show     "xhh:mm xhh:mm"
    lcd.write(CHAR_ARROW_UP);
    lcd.print(leftFill(String(storage.switch_times[index][0]), 2, "0") + ":" + 
              leftFill(String(storage.switch_times[index][1]), 2, "0") + "  ");
    lcd.write(CHAR_ARROW_DOWN);
    lcd.print(leftFill(String(storage.switch_times[index + 1][0]), 2, "0") + ":" + 
              leftFill(String(storage.switch_times[index + 1][1]), 2, "0") + " ");
    
    lcd.setCursor(cursPos_ts[cursor_index_ts_set], 1);
}


void handleKeystroke_set_time_switch(byte index) {
    index--;
    lcd.cursor();
    lcd.blink();
    
    switch (kbdValue) {
    case KEY_ENTER:   // enter (key 0 in set date time mode
        // finish
        displayMode = 0;
        kbdValue = 255; 
        lcd.noCursor();
        lcd.noBlink();
        storeEEprom();
        break;
      
    case KEY_RIGHT: // right (key 1 in set date time mode
        cursor_index_ts_set = increment_byte_value(cursor_index_ts_set, 0, SET_TS_CURSOR_POS_CNT - 1, 1);
        break;
    case KEY_LEFT: // left (key 2 in set date time mode
        cursor_index_ts_set = decrement_byte_value(cursor_index_ts_set, 0, SET_TS_CURSOR_POS_CNT - 1, 1);
        break;
        
    case KEY_UP: // up 
        switch (cursor_index_ts_set) {
            // up switch time
            case 0: 
                storage.switch_times[index][0] = increment_byte_value(storage.switch_times[index][0], 0, 23, 1);
                break;
            case 1:
                storage.switch_times[index][1] = increment_byte_value(storage.switch_times[index][1], 0, 55, 5);
                break;
            // down switch time
            case 2: 
                storage.switch_times[index+1][0] = increment_byte_value(storage.switch_times[index+1][0], 0, 23, 1);
                break;
            case 3:
                storage.switch_times[index+1][1] = increment_byte_value(storage.switch_times[index+1][1], 0, 55, 5);
                break;
        }
        break;

    case KEY_DOWN: // down 
        switch (cursor_index_ts_set) {
            //hour
            case 0: 
                storage.switch_times[index][0] = decrement_byte_value(storage.switch_times[index][0], 0, 24, 1);
                break;
            case 1:
                storage.switch_times[index][1] = decrement_byte_value(storage.switch_times[index][1], 0, 55, 5);
                break;
            // down switch time
            case 2: 
                storage.switch_times[index+1][0] = decrement_byte_value(storage.switch_times[index+1][0], 0, 24, 1);
                break;
            //minute
            case 3:
                storage.switch_times[index+1][1] = decrement_byte_value(storage.switch_times[index+1][1], 0, 55, 5);
                break;
        }
        break;

        case KEY_LEFTUP:
            displayMode = decrement_byte_value(displayMode, 0, 3, 1); // flip through the pages
            resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
            break;

        case KEY_LEFTDOWN:
            displayMode = increment_byte_value(displayMode, 0, 3, 1); // flip through the pages
            resetDisplayModeSeconds = tmh.getSecondsCounter() + 15;
            break;
                
                    
    default:
      break;
    
    }
}
  
