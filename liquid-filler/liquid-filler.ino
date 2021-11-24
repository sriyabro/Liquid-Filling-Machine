// PINS
const int waterFlowSensorPin = 2; // Water flow sensor
const int startFill = A0;         // Start fill Button
const int pumpRelay = A1;         // Pump ON/OFF relay

const int stepperPulse = 11;
const int stepperDir = 12;
const int upButton = A2;
const int downButton = A3;

const int _250 = 4;
const int _500 = 5;
const int _750 = 6;
const int _1000 = 9;
const int _2000 = 8;
const int _2500 = 7;
const int _5000 = 10;

const int nozzelLimit = 3; // Limit switch at nozzel
const int topLimit = A6;
const int bottomLimit = A7;

const int SDA = A4;
const int SCL = A5;

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
bool topLimitReached = false;
bool bottomLimitReached = false;

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
  pinMode(topLimit, INPUT_PULLUP);
  pinMode(bottomLimit, INPUT_PULLUP);

  digitalWrite(pumpRelay, HIGH); // Turn OFF pump

  Serial.begin(9600);
}

//--------------------------------------------------------------------------
// LOOP
void loop()
{
  checkStepper();
  getVolumeToFill();

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
        Serial.println("Start Fill Button Pressed !!");
        startFilling();
      }
    }
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
    if (millis() > (startTime + 1000)) // calculate filled volume every second
    {
      filledVolume = calculateFilledVolume();
      Serial.print("Filled Volume: ");
      Serial.println(filledVolume);

      if (filledVolume >= volumeToFill) // if bottle is filled with selected volume
      {
        digitalWrite(pumpRelay, HIGH); // Turn OFF the pump
        Serial.println("Pump OFF");

        Serial.print("Bottle filled !! - ");
        Serial.println(filledVolume);

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
  flowRate = ((pulseCount * 60 / calibrationFactor) * 1000) / 3600; // flow rate in ml/sec

  flowMilliLitres = flowRate * ((millis() - startTime) / 1000); // milliliters flown from attach to detach (sec loop start to now)

  totalMilliLitres += flowMilliLitres;

  pulseCount = 0;
  flowMilliLitres = 0;
  startTime = millis();

  return totalMilliLitres;
}

// get volume to fill before start filling
void getVolumeToFill()
{
  int prevVolumeToFill = volumeToFill;
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

  if (volumeToFill != prevVolumeToFill)
  {
    Serial.print("Fill Volume: ");
    Serial.println(volumeToFill);
  }
}

void checkStepper()
{
  while (digitalRead(downButton) == LOW || digitalRead(upButton) == LOW)
  {
    checkLimitSwitches();
    if (!topLimitReached || !bottomLimitReached)
    {
      return;
    }
    else
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
  if (digitalRead(topLimit) == LOW)
  {
    topLimitReached = true;
  }
  else
  {
    topLimitReached = false;
  }

  if (digitalRead(bottomLimit) == LOW)
  {
    bottomLimitReached = true;
  }
  else
  {
    bottomLimitReached = false;
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

// Interrupt SRs
void pulseCounter() // pulse counter for water flow sensor
{
  pulseCount++;
}
