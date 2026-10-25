// Menu for set the switch points for the timer



#define SET_TP_CURSOR_POS_CNT 2

byte cursor_index_tp_set = 0;
const byte curs_pos_tp[SET_TP_CURSOR_POS_CNT] = {3, 9};

void display_set_temp_limit() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Temperaturen:"); 
    lcd.setCursor(0, 1);
    lcd.print("  "); 
    lcd.print(leftFill(String(storage.temp_min), 2, "0") + "\xdf   ");
    lcd.print(leftFill(String(storage.temp_max), 2, "0") + "\xdf");
    
    lcd.setCursor(curs_pos_tp[cursor_index_tp_set], 1);
}


void handle_keystroke_set_temp_limit() {
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
        cursor_index_tp_set = increment_byte_value(cursor_index_tp_set, 0, SET_TP_CURSOR_POS_CNT - 1, 1);
        break;
    case KEY_LEFT: // left (key 2 in set date time mode
        cursor_index_tp_set = decrement_byte_value(cursor_index_tp_set, 0, SET_TP_CURSOR_POS_CNT - 1, 1);
        break;
        
    case KEY_UP: // up 
        switch (cursor_index_tp_set) {
            // up switch time
            case 0: 
                storage.temp_min = increment_byte_value(storage.temp_min, 16, 22, 1);
                break;
            case 1:
                storage.temp_max = increment_byte_value(storage.temp_max, 24, 26, 1);
                break;
       }
        break;

    case KEY_DOWN: // down 
        switch (cursor_index_tp_set) {
            case 0: 
                storage.temp_min = decrement_byte_value(storage.temp_min, 16, 22, 1);
                break;
            case 1:
                storage.temp_max = decrement_byte_value(storage.temp_max, 24, 26, 1);
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
  
