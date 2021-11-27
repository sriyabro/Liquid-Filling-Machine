/**
  -- PINOUT
  -- SENSOR PINS
  waterFlowSensorPin - D2 - PD2 (INT0)   // Water flow sensor
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
  1000 ml - D9 - PB1
  2000 ml - D8 - PB0
  2500 ml - D7 - PD7
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
const int waterFlowSensorPin = 2;        // Water flow sensor
const int startFill = A0;                // Start fill Button
const int pumpRelay = A1;                // Pump ON/OFF relay

// STEPPER MOTOR CONTROLS
const int stepperPulse = 11;
const int stepperDir = 12;
const int upButton = A2;
const int downButton = A3;

// VOLUME SELECTOR BUTTONS
const int _250 = 4;
const int _500 = 5;
const int _750 = 6;
const int _1000 = 9;
const int _2000 = 8;
const int _2500 = 7;
const int _5000 = 10;

// LIMIT SWITCHES
const int nozzelLimit = 3;               // Limit switch at nozzel
const int topBottomLimit = 13;           // Limit switches at top & bottom

// Global variables
float calibrationFactor = 0;

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
  pinMode(waterFlowSensorPin, INPUT_PULLUP);

  pinMode(stepperPulse, OUTPUT);
  pinMode(stepperDir, OUTPUT);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);

  pinMode(nozzelLimit, INPUT_PULLUP);
  pinMode(topBottomLimit, INPUT_PULLUP);

  digitalWrite(pumpRelay, HIGH);         // Turn OFF pump

  lcd.begin();
  lcd.backlight();
  printStrToLCD("Power ON", 1);
  startUpMesaage();                      // [TODO] - Complete Startup message

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
  attachInterrupt(digitalPinToInterrupt(waterFlowSensorPin), pulseCounter, RISING);
  digitalWrite(pumpRelay, LOW);          // Turn ON the relay
  startTime = millis();

  while (filledVolume < volumeToFill)
  {
    if (millis() > (startTime + 1000))   // Calculate filled volume every second
    {
      filledVolume = calculateFilledVolume();
      printStrToLCD("FILLED VOLUME: ", 1);
      printVarToLCD(filledVolume, 2);
      lcd.print(" / ");
      lcd.print(volumeToFill);
      lcd.print(" ml");

      if (filledVolume >= volumeToFill)  // if bottle is filled with selected volume
      {
        digitalWrite(pumpRelay, HIGH);   // Turn OFF the pump

        Serial.print("Bottle filled !! - "); // [TODO] - Remove  after debug
        Serial.println(filledVolume);

        lcd.clear();
        printStrToLCD("BOTTLE FILLED", 1);
        printVarToLCD(volumeToFill, 2);
        lcd.print(" ml");

        moveNozzelUp();
        nozzelLimitReached = false;

        pulseCount = 0;
        detachInterrupt(digitalPinToInterrupt(waterFlowSensorPin));
        startFillPressed = false;
        filledVolume = 0.0;
        totalMilliLitres = 0.0;

        break; // back to loop
      }
    }
  }
}

long calculateFilledVolume()
{
  flowRate = ((pulseCount * 60 / calibrationFactor) * 1000) / 3600;     // flow rate in ml/sec

  flowMilliLitres = flowRate * ((millis() - startTime) / 1000);         // milliliters flown from attach to detach (sec loop start to now)

  totalMilliLitres += flowMilliLitres;

  pulseCount = 0;
  flowMilliLitres = 0;
  startTime = millis();

  return totalMilliLitres;
}

// Get volume to fill before start filling
void getVolumeToFill()
{
  int prevVolumeToFill = volumeToFill;
  if (digitalRead(_250) == LOW)
  {
    volumeToFill = 250;
    calibrationFactor = 6.2;             // [TODO] - calibarate per volume
  }
  else if (digitalRead(_500) == LOW)
  {
    volumeToFill = 500;
    calibrationFactor = 6.2;             // [TODO] - calibarate per volume
  }
  else if (digitalRead(_750) == LOW)
  {
    volumeToFill = 750;
    calibrationFactor = 6.2;             // [TODO] - calibarate per volume
  }
  else if (digitalRead(_1000) == LOW)
  {
    volumeToFill = 1000;
    calibrationFactor = 6.2;             // [TODO] - calibarate per volume
  }
  else if (digitalRead(_2000) == LOW)
  {
    volumeToFill = 2000;
    calibrationFactor = 6.2;             // [TODO] - calibarate per volume
  }
  else if (digitalRead(_2500) == LOW)
  {
    volumeToFill = 2500;
    calibrationFactor = 6.2;             // [TODO] - calibarate per volume
  }
  else if (digitalRead(_5000) == LOW)
  {
    volumeToFill = 5000;
    calibrationFactor = 6.2;             // [TODO] - calibarate per volume
  }

  if (volumeToFill != prevVolumeToFill)
  {
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
      delayMicroseconds(200);
      digitalWrite(stepperPulse, LOW);
      delayMicroseconds(200);
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
  for (int i = 0; i < 5000; i++)
  {
    digitalWrite(stepperPulse, HIGH);
    delayMicroseconds(200);
    digitalWrite(stepperPulse, LOW);
    delayMicroseconds(200);
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
    delayMicroseconds(200);
    digitalWrite(stepperPulse, LOW);
    delayMicroseconds(200);
  }
  if (digitalRead(nozzelLimit) == LOW)
  {
    nozzelLimitReached = true;
  }
}

void safeReturn()
{
  digitalWrite(stepperDir, !digitalRead(stepperDir));
  for (int i = 0; i < 2500; i++)
  {
    digitalWrite(stepperPulse, HIGH);
    delayMicroseconds(200);
    digitalWrite(stepperPulse, LOW);
    delayMicroseconds(200);
  }
  delay(1000);
}

// Interrupt SRs
void pulseCounter()                      // pulse counter for water flow sensor
{
  pulseCount++;
}

// print String to LCD
void printStrToLCD(String text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                ");         // clear line before print
  lcd.setCursor(0, lineNo - 1);
  lcd.print(text);
}

// print variable to LCD
void printVarToLCD(int text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                ");         // clear line before print
  lcd.setCursor(0, lineNo - 1);
  lcd.print(text);
}

void startUpMesaage()
{
  lcd.clear();
  /*  STARTUP MESSAGE HERE  */
}