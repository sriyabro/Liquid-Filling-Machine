#include <Keypad.h>

const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns

// key map
char hexaKeys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};

byte rowPins[ROWS] = {26, 27, 28, 29};
byte colPins[COLS] = {30, 31, 32, 33};

// initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

void setup()
{
  Serial.begin(9600);
}

void loop()
{
  char customKey = customKeypad.getKey();

  if (customKey)
  {
    Serial.println(customKey);
  }
}

/** KEY MAP
 
    1 - '1'
    2 - '2'
    3 - '3'
    4 - '4'
    5 - '5'
    6 - '6'
    7 - '7'
    8 - '8'
    9 - '9'
    0 - '0'
    # - '#'
    BACK - '*'
    UP - 'A'
    DOWN - 'B'
    CLEAR - 'C'
    OK - 'D'

 */