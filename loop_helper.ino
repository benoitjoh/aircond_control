
#define TEMP_HYSTERESIS 0.5

void toggle_backlight(byte alarm_on) {
    if ( alarm_on ) {
        backlight = !backlight;
        }
    else {
        backlight = 1;
        }

    setBacklight(backlight);
}

// decide by the temparatur limits and the actual temp which mode is neccessary
void set_function_by_temp_condition() {
    float mid_temp = storage.temp_min + (storage.temp_max - storage.temp_min) / 2.0;
    if ( engine_state == 1 ) {
        if ( temp < mid_temp ) {
            function_state = 2;
            valve_state = 1;
        }
        else {
            function_state = 1;
            valve_state = 0;
        }
    }
    else {
        function_state = 0;
        valve_state = 0;
    }
}


void set_compressor_by_temp_condition() {
    if ( engine_state == 1 ) {
        if ( temp + TEMP_HYSTERESIS < storage.temp_min ||
             temp - TEMP_HYSTERESIS > storage.temp_max ) {
                if (compressor_state == 0) {
                    request_start_compressor();
                }
        }
        else {
            if ( temp - TEMP_HYSTERESIS > storage.temp_min ||
                temp + TEMP_HYSTERESIS < storage.temp_max ) {
                    if (compressor_state == 1) {
                        request_stop_compressor();
                    }
                }
        }       
    }
}


void request_start_compressor() {
    pump_state = 1; 
    fan_state = 1;
    compressor_state = 1;
}


void request_stop_compressor() {
    compressor_state = 0;
    pump_state = 2; 
    fan_state = 2;
    pump_wait = PUMP_WAIT_SECS;
    function_state = 0;
    valve_state = 0;
}
