// Menu for set the switch points for the timer



#define SET_TS_CURSOR_POS_CNT 4

byte cursor_index_ts_set = 1;
const byte cursPos_ts[SET_TS_CURSOR_POS_CNT] = {2, 5, 9, 13};

void display_set_switch_time(byte index) {
    // index is 1 or 2. to adress values in array 0, 1 reduce by it by 1
    index--;
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Schaltzeit " + String(index) + ":"); 
    lcd.setCursor(0, 1);
    /// pos         2  5   9  c
    /// show     "xhh:mm xhh:mm"
    lcd.print(CHAR_ARROW_UP);
    lcd.print(leftFill(String(storage.switch_times[index][0]), 2, "0") + ":" + 
              leftFill(String(storage.switch_times[index][1]), 2, "0") + " ");
    lcd.print(CHAR_ARROW_DOWN);
    lcd.print(leftFill(String(storage.switch_times[index + 1][0]), 2, "0") + ":" + 
              leftFill(String(storage.switch_times[index + 1][1]), 2, "0") + " ");
    
    lcd.setCursor(cursPos_ts[cursor_index_ts_set], 1);
}

byte increment_value(byte value, byte max, byte step) {
    value += step;
    if ( value > max ) {value = 0;}
    return value;
    }

byte decrement_value(byte value, byte max, byte step) {
    value -= step;
    if ( value > max ) {value = max;}
    return value;
    }


void handleKeystroke_set_switch_time(byte index) {
    index++;
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
        cursor_index_ts_set++;
        if (cursor_index_ts_set >= SET_TS_CURSOR_POS_CNT) { cursor_index_ts_set = 1; }
        break;
    case KEY_LEFT: // left (key 2 in set date time mode
        cursor_index_ts_set--;
        if (cursor_index_ts_set < 1) { cursor_index_ts_set = SET_TS_CURSOR_POS_CNT-1; }
        break;
        
    case KEY_UP: // up 
        switch (cursor_index_ts_set) {
            // up switch time
            case 0: 
                storage.switch_times[index][0] = increment_value(storage.switch_times[index][0], 24, 1);
                break;
            case 1:
                storage.switch_times[index][1] = increment_value(storage.switch_times[index][1], 55, 5);
                break;
            // down switch time
            case 2: 
                storage.switch_times[index+1][0] = increment_value(storage.switch_times[index+1][0], 24, 1);
                break;
            case 3:
                storage.switch_times[index+1][1] = increment_value(storage.switch_times[index+1][1], 55, 5);
                break;
        }
        break;

    case KEY_DOWN: // down 
        switch (cursor_index_ts_set) {
            //hour
            case 0: 
                storage.switch_times[index][0] = decrement_value(storage.switch_times[index][0], 24, 1);
                break;
            case 1:
                storage.switch_times[index][1] = decrement_value(storage.switch_times[index][1], 55, 5);
                break;
            // down switch time
            case 2: 
                storage.switch_times[index+1][0] = decrement_value(storage.switch_times[index+1][0], 24, 1);
                break;
            //minute
            case 3:
                storage.switch_times[index+1][1] = decrement_value(storage.switch_times[index+1][1], 55, 5);
                break;
        }
        break;
                    
                    
    default:
      break;
    
    }
}
  
