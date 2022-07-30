#include <Keypad.h>

const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns

// key map
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

byte rowPins[ROWS] = {26, 27, 28, 29};
byte colPins[COLS] = {30, 31, 32, 33};

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

void setup(){
  Serial.begin(9600);
}
  
void loop(){
  char customKey = customKeypad.getKey();
  
  if (customKey){
    Serial.println(customKey);
  }
}

// 0 - 48
// 1 - 49
// 2 - 50
// 3 - 51
// 4 - 52
// 5 - 53
// 6 - 54
// 7 - 55
// 8 - 56
// 9 - 57
// * - 42 - back
// # - 35 - 
// A - 65 - up
// B - 66 - down
// C - 67 - clear
// D - 68 - OK


// 250, 330, 500, 675, 750, 1000, 2000 and 5000