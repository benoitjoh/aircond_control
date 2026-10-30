

void show_page_debug_advalues() {
    // -- display a0 a1 a2 a3
    lcd.setCursor(0,0);
    lcd.print("  A0  A1  A2  A3");

    lcd.setCursor(0,1);
    lcd.print(leftFill(String(analogRead(0)), 4, " ") +
    leftFill(String(analogRead(1)), 4, " ") + 
    leftFill(String(analogRead(2)), 4, " ") + 
    leftFill(String(analogRead(3)), 4, " ") );
}


void show_page_standard() {
    // -- display normal information each second
    //     [20-25°] kü  VKP   [temp range], kü|hz, Ventilator, Kompressor, Pumpe 
    //     20.2° AUTO 12:23   akt. temp, AUTO/EIN/AUS, Time
    lcd.setCursor(0,0);
    lcd.print("[" + leftFill(String(storage.temp_min), 2, " ") + "-" +
    leftFill(String(storage.temp_max), 2, " ") + "] ");
    lcd.write(function_state_name[function_state][0]);
    lcd.write(function_state_name[function_state][1]);
    lcd.print("   ");
    lcd.write(fan_state_name[fan_state]);
    lcd.write(compressor_state_name[compressor_state]);
    lcd.write(pump_state_name[pump_state]);

    lcd.setCursor(0,1);
    lcd.print(leftFill(String(temp, 1), 4, " ") + "\xdf" + " ");
    lcd.print(run_mode_name[run_mode] + " ");
    lcd.print(tmh.getHrsMinSec());
}



