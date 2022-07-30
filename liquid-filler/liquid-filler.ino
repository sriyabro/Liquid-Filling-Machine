/**
 * @file liquid-filler.ino
 * @author Sriyanjith Herath (sriyabro@gmail.com)
 * @brief Automated liquid filler machine firmware for the Arduino Mega 2560 R3 .
 * @version 4.1
 * @date 2022-07-30
 *
 * @copyright  Copyright 2021 Sriyanjith Herath. All rights reserved. Unauthorized access, copying, publishing, sharing, reuse of algorithms, concepts, design patterns and code level demonstrations are strictly prohibited without any written approval of the author.
 */

/**
  -- PINOUT
  -- SENSOR PINS
  startFill - A0 - PF0                   // Start fill Button
  pumpRelay - A1 - PF1                   // Pump ON/OFF relay

  -- STEPPER MOTOR CONTROLS
  stepperPulse - D11 - PB5
  stepperDir - D12 - PB6 - (HIGH - Up | LOW - Down)
  upButton - A2 - PF2
  downButton - A3 - PF3

  -- FILLING VOLUME SELECTOR BUTTONS
  250 ml - D4 - PG5
  330 ml - D5 - PE3
  500 ml - D6 - PH3
  675 ml - D7 - PH4
  750 ml - D8 - PH5
  1000 ml - D9 - PH6
  2000 ml - D10 - PB4
  5000 ml - D24 - PA2

  -- LIMIT SWITCHES
  nozzelLimit - D3 - PE5                 // Limit switch at nozzel
  topBottomLimit - D13 - PB7             // Limit switches at top & bottom

  -- LCD PINS
  SDA - A4 - PF4
  SCL - A5 - PF5

  -- PERISTALTIC PUMP STEPPER CONTROLS
  pumpPulse - D22 - PA0
  pumpDir - D23 - PA1

  -- MANUAL FILL BUTTON
  manualFillBtn - D25 - PA3

  -- KEYPAD 4x4
  ROWS - D26, D27, D28, D29
  Cols - D30, D31, D32, D33

-- MENU INVOKE BUTTON
  menuBtn - D19 - PD2

-- KEYPAD BUTTON MAP
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

**/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

/*--------------------------------------------------------------------------
 * SET PINS
 */
// SENSOR PINS
const int startFill = A0;          // Start fill Button
const int solenoidValveRelay = A1; // Solenoid valve ON/OFF relay

// PERISTALTIC PUMP STEPPER PINS
const int pumpPulse = 22; // Pump step pin
const int pumpDir = 23;   // Pump direction pin

// STEPPER MOTOR CONTROL PINS
const int stepperPulse = 11;
const int stepperDir = 12;
const int upButton = A2;
const int downButton = A3;

// VOLUME SELECTOR BUTTONS PINS
const int _250 = 4;
const int _330 = 5;
const int _500 = 6;
const int _675 = 7;
const int _750 = 8;
const int _1000 = 9;
const int _2000 = 10;
const int _5000 = 24;

// MANUAL FILL BTN PIN
const int manualFillBtn = 25; // manual fill button - press hold to fill

// LIMIT SWITCHES PINS
const int nozzelLimit = 3;     // Limit switch at nozzel
const int topBottomLimit = 13; // Limit switches at top & bottom

// KEYPAD PINS
byte rowPins[4] = {26, 27, 28, 29};
byte colPins[4] = {30, 31, 32, 33};

// MENU BUTTON PIN
const int menuBtn = 19; // menu button

/*--------------------------------------------------------------------------
 * INITAILIZE VARIABLES
 */
// Constants
const int stepperPulseDelayMicros = 150; // Stepper motor pulse delay in microseconds
const int moveNozzelUpSteps = 5000;
const int safeReturnMoveSteps = 2500;

const int addressSpacingFactor = 8; // EEPROM address spacing factor

const int pumpPulseDelay = 150; // Pump pulse delay in microseconds

// Global variables
float volPerSec = 0.0; // Volume per second

double pumpSteps = 0; // Number of steps to move the pump

unsigned int volumeToFill = 0;
bool startFillPressed = false;

volatile unsigned int pulseCount = 0;

float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;

float filledVolume = 0.0;

unsigned long startTime = 0;

bool nozzelLimitReached = false;
bool limitReached = false;

// key map
char keyMap[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
Keypad keypad = Keypad(makeKeymap(keyMap), rowPins, colPins, 4, 4);

String menuItems[] = {"250 ml", "330 ml", "500 ml", "675 ml", "750 ml", "1000 ml", "2000 ml", "5000 ml"};

bool menuInvoked = false;
// Navigation button variables
int readKey;
int key;
// Menu control variables
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;
int result = 0;

String inputString = ""; // number input string

// Custom characters for the menu display
byte downArrow[8] = {
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b10101, // * * *
    0b01110, //  ***
    0b00100  //   *
};

byte upArrow[8] = {
    0b00100, //   *
    0b01110, //  ***
    0b10101, // * * *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100  //   *
};

byte menuCursor[8] = {
    B01000, //  *
    B00100, //   *
    B00010, //    *
    B00001, //     *
    B00010, //    *
    B00100, //   *
    B01000, //  *
    B00000  //
};
/*--------------------------------------------------------------------------
 * SETUP
 */
void setup()
{
  pinMode(solenoidValveRelay, OUTPUT);

  pinMode(_250, INPUT_PULLUP);
  pinMode(_330, INPUT_PULLUP);
  pinMode(_500, INPUT_PULLUP);
  pinMode(_675, INPUT_PULLUP);
  pinMode(_750, INPUT_PULLUP);
  pinMode(_1000, INPUT_PULLUP);
  pinMode(_2000, INPUT_PULLUP);
  pinMode(_5000, INPUT_PULLUP);

  pinMode(stepperPulse, OUTPUT);
  pinMode(stepperDir, OUTPUT);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);

  pinMode(pumpPulse, OUTPUT);
  pinMode(pumpDir, OUTPUT);

  pinMode(startFill, INPUT_PULLUP);     // start fill btn
  pinMode(manualFillBtn, INPUT_PULLUP); // manual fil btn

  pinMode(nozzelLimit, INPUT_PULLUP);
  pinMode(topBottomLimit, INPUT_PULLUP);

  // initailize pins
  digitalWrite(solenoidValveRelay, HIGH); // Turn OFF pump
  digitalWrite(pumpDir, LOW);             // Set pump direction - Change HIGH to LOW to move the pump in the opposite direction

  lcd.begin();
  lcd.backlight();
  startUpMesaage();

  Serial.begin(9600);

  keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);

  pinMode(menuBtn, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(menuBtn), setMenuInvoke, LOW);
}

/*--------------------------------------------------------------------------
 * LOOP
 */
void loop()
{
  getVolumeToFill();
  checkStepper();
  if (volumeToFill)
  {
    if (digitalRead(startFill) == LOW)
    {
      startFillPressed = true;
    }
    if (startFillPressed && volumeToFill)
    {
      moveNozzelDown();

      if (startFillPressed && volumeToFill && nozzelLimitReached)
      {
        startFilling();
      }
    }
    else if (digitalRead(manualFillBtn) == LOW)
    {
      moveNozzelDown();
      if (digitalRead(manualFillBtn) == LOW && nozzelLimitReached)
      {
        runManualFill();
      }
    }
  }
  if (menuInvoked)
  {
    menu();
  }
}

void startFilling()
{
  digitalWrite(solenoidValveRelay, LOW); // Turn ON the relay
  for (int i = 0; i <= pumpSteps; i++)
  {
    digitalWrite(pumpPulse, HIGH);
    delayMicroseconds(pumpPulseDelay);
    digitalWrite(pumpPulse, LOW);
    delayMicroseconds(pumpPulseDelay);
  }
  digitalWrite(solenoidValveRelay, HIGH); // Turn OFF the relay

  lcd.clear();
  printStrToLCD("BOTTLE FILLED", 1);
  printVarToLCD(volumeToFill, 2);
  lcd.print(" ml");

  nozzelLimitReached = false;
  moveNozzelUp();
  startFillPressed = false;
}

void runManualFill()
{
  digitalWrite(solenoidValveRelay, LOW); // Turn ON the relay
  while (digitalRead(manualFillBtn) == LOW)
  {
    digitalWrite(pumpPulse, HIGH);
    delayMicroseconds(pumpPulseDelay);
    digitalWrite(pumpPulse, LOW);
    delayMicroseconds(pumpPulseDelay);
    if (digitalRead(manualFillBtn) == HIGH)
    {
      break;
    }
  }
  digitalWrite(solenoidValveRelay, HIGH); // Turn OFF the relay
  nozzelLimitReached = false;
  moveNozzelUp();
}

// Get volume to fill before start filling
void getVolumeToFill()
{
  unsigned int prevVolumeToFill = volumeToFill;
  if (digitalRead(_250) == LOW)
  {
    volumeToFill = 250;
    volPerSec = 29.7;
    EEPROM.get(0 * addressSpacingFactor, pumpSteps);
  }
  else if (digitalRead(_330) == LOW)
  {
    volumeToFill = 330;
    volPerSec = 29.5;
    EEPROM.get(1 * addressSpacingFactor, pumpSteps);
  }
  else if (digitalRead(_500) == LOW)
  {
    volumeToFill = 500;
    volPerSec = 29.4;
    EEPROM.get(2 * addressSpacingFactor, pumpSteps);
  }
  else if (digitalRead(_675) == LOW)
  {
    volumeToFill = 675;
    volPerSec = 30.5;
    EEPROM.get(3 * addressSpacingFactor, pumpSteps);
  }
  else if (digitalRead(_750) == LOW)
  {
    volumeToFill = 750;
    volPerSec = 32.8;
    EEPROM.get(4 * addressSpacingFactor, pumpSteps);
  }
  else if (digitalRead(_1000) == LOW)
  {
    volumeToFill = 1000;
    volPerSec = 36.4;
    EEPROM.get(5 * addressSpacingFactor, pumpSteps);
  }
  else if (digitalRead(_2000) == LOW)
  {
    volumeToFill = 2000;
    volPerSec = 36.4;
    EEPROM.get(6 * addressSpacingFactor, pumpSteps);
  }
  else if (digitalRead(_5000) == LOW)
  {
    volumeToFill = 5000;
    volPerSec = 36.4;
    EEPROM.get(7 * addressSpacingFactor, pumpSteps);
  }

  if (volumeToFill != prevVolumeToFill)
  {
    lcd.clear();
    printStrToLCD("Selected Volume : ", 1);
    printVarToLCD(volumeToFill, 2);
    lcd.print(" ml");
  }
}

void checkStepper()
{
  while (digitalRead(downButton) == LOW || digitalRead(upButton) == LOW)
  {
    checkLimitSwitches();
    if (!limitReached)
    {
      if (digitalRead(downButton) == LOW)
      {
        digitalWrite(stepperDir, LOW);
      }
      else
      {
        digitalWrite(stepperDir, HIGH);
      }
      digitalWrite(stepperPulse, HIGH);
      delayMicroseconds(stepperPulseDelayMicros);
      digitalWrite(stepperPulse, LOW);
      delayMicroseconds(stepperPulseDelayMicros);
    }
  }
}

void checkLimitSwitches()
{
  if (digitalRead(topBottomLimit) == LOW || digitalRead(nozzelLimit) == LOW)
  {
    limitReached = true;
    safeReturn();
    limitReached = false;
  }
}

void moveNozzelUp()
{
  digitalWrite(stepperDir, HIGH);
  for (int i = 0; i <= moveNozzelUpSteps; i++)
  {
    digitalWrite(stepperPulse, HIGH);
    delayMicroseconds(stepperPulseDelayMicros);
    digitalWrite(stepperPulse, LOW);
    delayMicroseconds(stepperPulseDelayMicros);
  }
}

void moveNozzelDown()
{
  digitalWrite(stepperDir, LOW);
  while (digitalRead(nozzelLimit) == HIGH)
  {
    if (digitalRead(topBottomLimit) == LOW)
    {
      startFillPressed = false;
      safeReturn();
      break;
    }
    digitalWrite(stepperPulse, HIGH);
    delayMicroseconds(stepperPulseDelayMicros);
    digitalWrite(stepperPulse, LOW);
    delayMicroseconds(stepperPulseDelayMicros);
  }
  if (digitalRead(nozzelLimit) == LOW)
  {
    nozzelLimitReached = true;
  }
}

void safeReturn()
{
  digitalWrite(stepperDir, !digitalRead(stepperDir));
  for (int i = 0; i < safeReturnMoveSteps; i++)
  {
    digitalWrite(stepperPulse, HIGH);
    delayMicroseconds(stepperPulseDelayMicros);
    digitalWrite(stepperPulse, LOW);
    delayMicroseconds(stepperPulseDelayMicros);
  }
  delay(1000);
}

void setMenuInvoke()
{
  menuInvoked = true;
}

void menu()
{
  mainMenuDraw();
  drawCursor();
  operateMainMenu();
}

void keypadEvent(KeypadEvent key)
{
  switch (keypad.getState())
  {
    int readKey;
  case PRESSED:
    break;
  }
}

void mainMenuDraw()
{
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  lcd.setCursor(1, 2);
  lcd.print(menuItems[menuPage + 2]);
  lcd.setCursor(1, 3);
  lcd.print(menuItems[menuPage + 3]);
  if (menuPage == 0)
  {
    lcd.setCursor(15, 3);
    lcd.write(byte(2));
  }
  else if (menuPage > 0 and menuPage < maxMenuPages)
  {
    lcd.setCursor(15, 3);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
  else if (menuPage == maxMenuPages)
  {
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}

void drawCursor()
{
  for (int x = 0; x < 4; x++)
  { // Erases current cursor
    lcd.setCursor(0, x);
    lcd.print(" ");
  }

  if (menuPage % 2 == 0)
  {
    if (cursorPosition % 2 == 0)
    { // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0)
    { // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
  }
  if (menuPage % 2 != 0)
  {
    if (cursorPosition % 2 == 0)
    { // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    }
    if (cursorPosition % 2 != 0)
    { // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}

void operateMainMenu()
{
  int activeButton = 0;
  while (activeButton == 0)
  {
    int button;
    button = evaluateButton(readKey);
    if (button == 42)
    {
      menuInvoked = false;
      break;
    }

    switch (button)
    {
    case 0: // When button returns as 0 there is no action taken
      break;
    case 66: // down -------------------------------------------------------
      button = 0;
      if (menuPage % 2 == 0 and cursorPosition % 2 != 0)
      {
        menuPage = menuPage + 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      if (menuPage % 2 != 0 and cursorPosition % 2 == 0)
      {
        menuPage = menuPage + 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      cursorPosition = cursorPosition + 1;
      cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

      Serial.print("menuPage: ");       // [DEBUG]
      Serial.println(menuPage);         // [DEBUG]
      Serial.print("cursorPosition: "); // [DEBUG]
      Serial.println(cursorPosition);   // [DEBUG]

      mainMenuDraw();
      drawCursor();
      activeButton = 1;
      break;
    case 65: // up -------------------------------------------------------
      button = 0;
      if (menuPage == 0)
      {
        cursorPosition = cursorPosition - 1;
        cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
      }
      if (menuPage % 2 == 0 and cursorPosition % 2 == 0)
      {
        menuPage = menuPage - 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      if (menuPage % 2 != 0 and cursorPosition % 2 != 0)
      {
        menuPage = menuPage - 1;
        menuPage = constrain(menuPage, 0, maxMenuPages);
      }

      cursorPosition = cursorPosition - 1;
      cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));

      Serial.print("menuPage: ");       // [DEBUG]
      Serial.println(menuPage);         // [DEBUG]
      Serial.print("cursorPosition: "); // [DEBUG]
      Serial.println(cursorPosition);   // [DEBUG]

      mainMenuDraw();
      drawCursor();
      activeButton = 1;
      break;
    case 68: // This case will execute if the "OK" button is pressed
      button = 0;
      activeButton = 1;

      Serial.print("menuPage: ");       // [DEBUG]
      Serial.println(menuPage);         // [DEBUG]
      Serial.print("cursorPosition: "); // [DEBUG]
      Serial.println(cursorPosition);   // [DEBUG]

      menuItem(cursorPosition);
      mainMenuDraw();
      drawCursor();
      break;
    default:
      break;
    }
  }
}

void menuItem(int itemId) // itemId - cursorPointer
{                         // Function executes when you select a item from main menu
  int activeButton = 0;

  lcd.clear();
  double savedSteps;
  EEPROM.get(itemId * addressSpacingFactor, savedSteps);

  printStrToLCD("Enter calibaration ", 1);
  lcd.setCursor(0, 1);
  lcd.print("value for ");
  lcd.setCursor(10, 1);
  lcd.print(menuItems[itemId]);
  lcd.setCursor(0, 2);
  lcd.print("Steps:");
  lcd.setCursor(7, 2);
  lcd.print(savedSteps);

  while (activeButton == 0)
  {
    char key = keypad.getKey();
    if (key)
    {
      if (key == 'D')
      {
        if (inputString.length() > 0)
        {
          double saveVal = inputString.toDouble();
          // SAVING TO EEPROM
          EEPROM.put((itemId * addressSpacingFactor), saveVal);

          lcd.clear();
          printStrToLCD("Calibaration value", 1);
          printStrToLCD("saved successfully !", 2);
          delay(2000);
          inputString = ""; // clear input
          activeButton = 1;
        }
      }
      else if (key == 'C')
      {
        inputString = ""; // clear input
      }
      else if (key == '*')
      {
        inputString = ""; // clear input
        activeButton = 1;
      }
      if (key >= '0' && key <= '9')
      {
        inputString += key;
      }
      printStrToLCD(inputString, 4);
    }
  }
}

int evaluateButton(int x)
{
  int result = 0;
  int key = keypad.getKey();
  x = key;
  return x;
}

// LCD -----------------------------------------------
// print String to LCD
void printStrToLCD(String text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                    "); // Clear line before print
  lcd.setCursor(0, lineNo - 1);
  lcd.print(text);
}

// print variable to LCD
void printVarToLCD(int text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                    "); // clear line before print
  lcd.setCursor(0, lineNo - 1);
  lcd.print(text);
}

// create and print custom characters to LCD
void printCustomChar(uint8_t location, uint8_t charmap[], int row, int col)
{
  lcd.createChar(location, charmap);
  lcd.setCursor(col - 1, row - 1);
  lcd.write(location);
}

// STARTUP MESSAGE ---------------------------------------------------
// logo characters
byte q11[] = {
    B01111,
    B11111,
    B11000,
    B11001,
    B11011,
    B11011,
    B11011,
    B11011};

byte q12[] = {
    B11111,
    B11111,
    B00000,
    B11111,
    B11111,
    B00000,
    B00000,
    B00000};

byte q13[] = {
    B11110,
    B11111,
    B00011,
    B11011,
    B11011,
    B00011,
    B00011,
    B00011};

byte q21[] = {
    B11011,
    B11011,
    B11011,
    B11011,
    B11001,
    B11000,
    B11111,
    B01111};

byte q22[] = {
    B00000,
    B00000,
    B00000,
    B11111,
    B11111,
    B00000,
    B11111,
    B11111};

byte q23[] = {
    B00011,
    B00011,
    B00011,
    B11011,
    B11011,
    B00011,
    B11111,
    B11111};

byte q24[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11000,
    B11000};

byte n15[] = {
    B11111,
    B11111,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000};

byte n16[] = {
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};

byte n17[] = {
    B11110,
    B11111,
    B00111,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011};

byte n25[] = {
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000};

byte n26[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};

byte n27[] = {
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011,
    B00011};

byte c110[] = {
    B01111,
    B11111,
    B11100,
    B11000,
    B11000,
    B11000,
    B11000,
    B11000};

byte c111[] = {
    B11111,
    B11111,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000};

byte c210[] = {
    B11000,
    B11000,
    B11000,
    B11000,
    B11000,
    B11100,
    B11111,
    B01111};

byte c211[] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
    B11111};

// Power ON startup message
void startUpMesaage()
{
  lcd.clear();
  printCustomChar(0, q11, 1, 7);
  printCustomChar(2, q12, 1, 8);
  printCustomChar(3, q13, 1, 9);
  printCustomChar(4, q21, 2, 7);
  printCustomChar(5, q22, 2, 8);
  printCustomChar(6, q23, 2, 9);
  printCustomChar(7, q24, 2, 10);
  delay(800);
  lcd.clear();

  printCustomChar(8, n15, 1, 7);
  printCustomChar(9, n16, 1, 8);
  printCustomChar(10, n17, 1, 9);
  printCustomChar(11, n25, 2, 7);
  printCustomChar(12, n26, 2, 8);
  printCustomChar(13, n27, 2, 9);
  delay(800);
  lcd.clear();

  printCustomChar(14, c110, 1, 7);
  printCustomChar(15, c111, 1, 8);
  printCustomChar(0, c111, 1, 9);
  printCustomChar(1, c210, 2, 7);
  printCustomChar(2, c211, 2, 8);
  printCustomChar(3, c211, 2, 9);
  delay(800);
  lcd.clear();
  lcd.setCursor(4, 0);
  lcd.print("POWER ON");
}