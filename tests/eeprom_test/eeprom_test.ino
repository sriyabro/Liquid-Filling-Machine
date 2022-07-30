/*
   EEPROM Clear

   Sets all of the bytes of the EEPROM to 0.
   Please see eeprom_iteration for a more in depth
   look at how to traverse the EEPROM.

   This example code is in the public domain.
*/

#include <EEPROM.h>

void setup()
{
  // initialize the LED pin as an output.
  Serial.begin(9600);

  Serial.println("start");
  for (int i = 0; i < EEPROM.length(); i++)
  {
    EEPROM.read(i);
  }
  Serial.println("end");
}

void loop()
{
  /** Empty loop. **/
}