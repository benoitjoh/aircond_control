// commeon helper methods 

/*!
@abstract   fill a string from left with chars "fill_char" until it reaches wanted_len
            replace minus-symbol by the shorter variant CHAR_DIVISE
*/
String leftFill(String a, byte wanted_len, String fill_char) {
    // fills a string with letters from left side that the resulting length is reached
    while (a.length() < wanted_len) {
        a = fill_char + a;
        }
        
    for (byte i=0; i<wanted_len; i++) {
        if (a[i] == '-') {
            a[i] = CHAR_DIVISE; // the short divise
            }
        }
    return a;
    }



// --- check function if 235V is disconnected -----------------------------------
void checkAcDisconnected() {
    if ( !PCCtrl.acNetIsAlive() ) {   
        storeEEprom();
        lcd.setCursor(0, 1);
        lcd.print("Net disconnected.");
        delay(3000);
        }
}


/*!
@abstract   Increment a byte value by step.
    if it gets higher than max, set to min 
*/
byte increment_byte_value(byte value, byte min, byte max, byte step) {
    value += step;
    if ( value > max ) {value = min;}
    return value;
    }

/*!
@abstract   Decrement a byte value by step.
    if it gets lower than min, set to max 
*/
byte decrement_byte_value(byte value, byte min, byte max, byte step) {
    value -= step;
    if ( value > max or value < min ) {value = max;}
    return value;
    }

