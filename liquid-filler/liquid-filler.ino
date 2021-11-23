/**
*     -- PINOUT
*     Water Flow Sensor Pin - D2
*     Start Fill Button - D3          
*     Pump ON/OFF relay - A1
*
*     -- VOULUME BUTTONS
*     250 ml Button - D4
*     500 ml Button - D5
*     750 ml Button - D6
*     1000 ml Button - D9
*     2000 ml Button - D8
*     2500 ml Button - D7
*     5000 ml Button - D10
*   
*     -- LCD DISPLAY
*     SDA - A4
*     SCL - A5
*   
**/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

int waterFlowSensorPin = 2; // Water flow sensor
int startFill = 3;          // Start fill Button
int pumpRelay = A1;         // Pump ON/OFF relay

int _250 = 4;
int _500 = 5;
int _750 = 6;
int _1000 = 9;
int _2000 = 8;
int _2500 = 7;
int _5000 = 10;

float calibrationFactor = 0;

int volumeToFill = 0;
boolean startFillPressed = false;

volatile unsigned int pulseCount = 0;

float flowRate = 0.0;
unsigned int flowMilliLitres = 0;
unsigned long totalMilliLitres = 0;

float filledVolume = 0.0;

unsigned long startTime = 0;

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
  pinMode(startFill, INPUT);
  pinMode(waterFlowSensorPin, INPUT_PULLUP);

  digitalWrite(pumpRelay, HIGH);                                        // Turn OFF pump
  attachInterrupt(digitalPinToInterrupt(startFill), startFillInt, LOW); // attach interrupt for start fill button

  lcd.begin();
  lcd.backlight();
  printStrToLCD("Power ON", 1);
  delay(5000);

  Serial.begin(9600);
}

void loop()
{
  getVolumeToFill();
  Serial.print("Fill Volume: ");
  Serial.println(volumeToFill);
  if (!volumeToFill)
  {
    printStrToLCD("SELECT VOLUME", 1);
  }

  if (startFillPressed && volumeToFill)
  {
    Serial.println("Start Fill Button Pressed !!");
    printStrToLCD("START", 1);

    startFilling();
  }
}

void startFilling()
{
  attachInterrupt(digitalPinToInterrupt(waterFlowSensorPin), pulseCounter, RISING);
  digitalWrite(pumpRelay, LOW); // Turn ON the relay
  Serial.println("Pump ON");
  startTime = millis();

  while (filledVolume < volumeToFill)
  {
    if (millis() > (startTime + 1000)) // calculate filled voulme every second
    {
      filledVolume = calculateFilledVolume();
      Serial.print("Filled Volume: ");
      Serial.println(filledVolume);
      printStrToLCD("FILLED VOLUME: ", 1);
      printVarToLCD(filledVolume, 2);

      if (filledVolume >= volumeToFill) // if bottle is filled with selected volume
      {
        digitalWrite(pumpRelay, HIGH); // Turn OFF the pump
        Serial.println("Pump OFF");

        Serial.print("Bottle filled !! - ");
        Serial.println(filledVolume);
        lcd.clear();
        printStrToLCD("BOTTLE FILLED", 1);

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
  flowRate = ((pulseCount * 60 / calibrationFactor) * 1000) / 3600; // flow rate in ml/sec

  flowMilliLitres = flowRate * ((millis() - startTime) / 1000); // milliliters flown from attach to detach (sec loop start to now)

  totalMilliLitres += flowMilliLitres;

  pulseCount = 0;
  flowMilliLitres = 0;
  startTime = millis();

  return totalMilliLitres;
}

// Interrupt SRs
void pulseCounter()
{
  pulseCount++;
}

void startFillInt()
{
  Serial.println("Start Interrupt");
  if (volumeToFill != 0)
  {
    startFillPressed = true;
  }
  else
  {
    startFillPressed = false;
  }
}

// get volume to fill before start filling
void getVolumeToFill()
{
  if (digitalRead(_250) == LOW)
  {
    volumeToFill = 250;
    calibrationFactor = 6.2; // [TODO] - calibarate per volume
  }
  else if (digitalRead(_500) == LOW)
  {
    volumeToFill = 500;
    calibrationFactor = 6.2; // [TODO] - calibarate per volume
  }
  else if (digitalRead(_750) == LOW)
  {
    volumeToFill = 750;
    calibrationFactor = 6.2; // [TODO] - calibarate per volume
  }
  else if (digitalRead(_1000) == LOW)
  {
    volumeToFill = 1000;
    calibrationFactor = 6.2; // [TODO] - calibarate per volume
  }
  else if (digitalRead(_2000) == LOW)
  {
    volumeToFill = 2000;
    calibrationFactor = 6.2; // [TODO] - calibarate per volume
  }
  else if (digitalRead(_2500) == LOW)
  {
    volumeToFill = 2500;
    calibrationFactor = 6.2; // [TODO] - calibarate per volume
  }
  else if (digitalRead(_5000) == LOW)
  {
    volumeToFill = 5000;
    calibrationFactor = 6.2; // [TODO] - calibarate per volume
  }

  if (volumeToFill)
  {
    printStrToLCD("Selected Volume : ", 1);
    printVarToLCD(volumeToFill, 2);
    lcd.print(" ml");
  }
}

// print String to LCD
void printStrToLCD(String text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                "); // clear line before print
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