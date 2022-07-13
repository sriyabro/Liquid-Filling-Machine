/**
 * @file liquid-filler.ino
 * @author Sriyanjith Herath (sriyabro@gmail.com)
 * @brief Automated liquid filler machine firmware for the Arduino Nano.
 * @version 3.0
 * @date 2021-11-28
 * 
 * @copyright  Copyright 2021 Sriyanjith Herath. All rights reserved. Unauthorized access, copying, publishing, sharing, reuse of algorithms, concepts, design patterns and code level demonstrations are strictly prohibited without any written approval of the author.
 */

/**
  -- PINOUT
  -- SENSOR PINS
  startFill - A0 - PC0                   // Start fill Button
  pumpRelay - A1 - PC1                   // Pump ON/OFF relay

  -- STEPPER MOTOR CONTROLS
  stepperPulse - D11 - PB3
  stepperDir - D12 - PB4 - (HIGH - Up | LOW - Down)
  upButton - A2 - PC2
  downButton - A3 - PC3

  -- FILLING VOLUME SELECTOR BUTTONS
  250 ml - D4 - PD4
  500 ml - D5 - PD5
  750 ml - D6 - PD6
  1000 ml - D7 - PD7
  2000 ml - D8 - PB0
  2500 ml - D9 - PB1
  5000 ml - D10 - PB2

  -- LIMIT SWITCHES
  nozzelLimit - D3 - PD3                 // Limit switch at nozzel
  topBottomLimit - D13 - PB5             // Limit switches at top & bottom

  -- LCD PINS
  SDA - A4 - PC4
  SCL - A5 - PC5

**/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// SENSOR PINS
const int startFill = A0;         // Start fill Button
const int pumpRelay = A1;         // Pump ON/OFF relay

// STEPPER MOTOR CONTROLS
const int stepperPulse = 11;
const int stepperDir = 12;
const int upButton = A2;
const int downButton = A3;

// VOLUME SELECTOR BUTTONS
const int _250 = 4;
const int _500 = 5;
const int _750 = 6;
const int _1000 = 7;
const int _2000 = 8;
const int _2500 = 9;
const int _5000 = 10;

// LIMIT SWITCHES
const int nozzelLimit = 3;     // Limit switch at nozzel
const int topBottomLimit = 13; // Limit switches at top & bottom

// Constants
const int stepperPulseDelayMicros = 200; // Stepper motor pulse delay in microseconds
const int moveNozzelUpSteps = 5000;
const int safeReturnMoveSteps = 2500;

// Global variables
float volPerSec = 0.0;

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

void setup()
{
  pinMode(pumpRelay, OUTPUT);

  pinMode(_250, INPUT_PULLUP);
  pinMode(_500, INPUT_PULLUP);
  pinMode(_750, INPUT_PULLUP);
  pinMode(_1000, INPUT_PULLUP);
  pinMode(_2000, INPUT_PULLUP);
  pinMode(_2500, INPUT_PULLUP);
  pinMode(_5000, INPUT_PULLUP);
  pinMode(startFill, INPUT_PULLUP);

  pinMode(stepperPulse, OUTPUT);
  pinMode(stepperDir, OUTPUT);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);

  pinMode(nozzelLimit, INPUT_PULLUP);
  pinMode(topBottomLimit, INPUT_PULLUP);

  digitalWrite(pumpRelay, HIGH); // Turn OFF pump

  lcd.begin();
  lcd.backlight();
  printStrToLCD("Power ON", 1);
  startUpMesaage(); // [TODO] - Complete Startup message

  Serial.begin(9600);
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
  }
}


void startFilling()
{
  digitalWrite(pumpRelay, LOW); // Turn ON the relay

  delay((volumeToFill / volPerSec) * 1000); // Delay based on volume to fill and flow rate
  digitalWrite(pumpRelay, HIGH); // Turn OFF the relay


  lcd.clear();
  printStrToLCD("BOTTLE FILLED", 1);
  printVarToLCD(volumeToFill, 2);
  lcd.print(" ml");

  nozzelLimitReached = false;
  moveNozzelUp();
  startFillPressed = false;
}

// Get volume to fill before start filling
void getVolumeToFill()
{
  unsigned int prevVolumeToFill = volumeToFill;
  if (digitalRead(_250) == LOW)
  {
    volumeToFill = 250;
    volPerSec = 14.4;
  }
  else if (digitalRead(_500) == LOW)
  {
    volumeToFill = 500;
    volPerSec = 14.4;
  }
  else if (digitalRead(_750) == LOW)
  {
    volumeToFill = 750;
    volPerSec = 14.4;
  }
  else if (digitalRead(_1000) == LOW)
  {
    volumeToFill = 1000;
    volPerSec = 14.4;
  }
  else if (digitalRead(_2000) == LOW)
  {
    volumeToFill = 2000;
    volPerSec = 14.4;
  }
  else if (digitalRead(_2500) == LOW)
  {
    volumeToFill = 2500;
    volPerSec = 14.4;
  }
  else if (digitalRead(_5000) == LOW)
  {
    volumeToFill = 5000;
    volPerSec = 14.4;
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

// print String to LCD
void printStrToLCD(String text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                "); // Clear line before print
  lcd.setCursor(0, lineNo - 1);
  lcd.print(text);
}

// print variable to LCD
void printVarToLCD(int text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                "); // clear line before print
  lcd.setCursor(0, lineNo - 1);
  lcd.print(text);
}

void startUpMesaage()
{
  lcd.clear();
  /*  STARTUP MESSAGE HERE  */
}