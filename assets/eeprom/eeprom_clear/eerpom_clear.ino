/*
 * EEPROM Clear
 *
 * Sets all of the bytes of the EEPROM to 0.
 * Please see eeprom_iteration for a more in depth
 * look at how to traverse the EEPROM.
 *
 * This example code is in the public domain.
 */

#include <EEPROM.h>

void setup()
{
    // initialize the LED pin as an output.
    pinMode(13, OUTPUT);
    Serial.begin(9600);

    /***
      Iterate through each byte of the EEPROM storage.
      Larger AVR processors have larger EEPROM sizes, E.g:
      - Arduino Duemilanove: 512 B EEPROM storage.
      - Arduino Uno:         1 kB EEPROM storage.
      - Arduino Mega:        4 kB EEPROM storage.
      Rather than hard-coding the length, you should use the pre-provided length function.
      This will make your code portable to all AVR processors.
    ***/

    for (int i = 0; i < EEPROM.length(); i++)
    {
        EEPROM.write(i, 0);
    }

    // blink the LED 3 times when done
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(13, HIGH);
        delay(1000);
        digitalWrite(13, LOW);
        delay(2000);
    }
    Serial.println("EEPROM CLEARED");
}

void loop()
{
    /** Empty loop. **/
}