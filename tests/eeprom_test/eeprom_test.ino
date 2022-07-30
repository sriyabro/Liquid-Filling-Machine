/*
   EEPROM Clear

   Sets all of the bytes of the EEPROM to 0.
   Please see eeprom_iteration for a more in depth
   look at how to traverse the EEPROM.

   This example code is in the public domain.
*/

#include <EEPROM.h>

String menuItems[] = {"250 ml", "330 ml", "500 ml", "675 ml", "750 ml", "1000 ml", "2000 ml", "5000ml"};
const int addressSpace = 8;
int value; 

void setup()
{
  Serial.begin(9600);
  String vaaaaal = "454644";
  double ssval = vaaaaal.toDouble();
  EEPROM.put(100, ssval);
  delay(300);

  Serial.println("read");
  double val;
  EEPROM.get(100, val);
  Serial.println(val);
}

void loop()
{
  /** Empty loop. **/
}

// WRITE
  // for (unsigned int i = 0; i < (sizeof(menuItems) / sizeof(String)); i++)
  // {
  //   value = i*200;
  //   Serial.println(value);
  //   EEPROM.put(i * addressSpace, value);

  //   delay(100);
  // }


  // // READ
  // for (unsigned int j = 0; j < (sizeof(menuItems) / sizeof(String)); j++)
  // {
  //   Serial.print("Saved:");
  //   Serial.println(EEPROM.get(j * addressSpace, value));
  //   delay(100);

  // }