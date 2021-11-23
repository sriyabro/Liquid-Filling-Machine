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

const float calibrationFactor = 6.2;

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

  digitalWrite(pumpRelay, HIGH); // Turn OFF pump
  attachInterrupt(digitalPinToInterrupt(startFill), startFillInt, LOW);

  Serial.begin(9600);
}

void loop()
{
  getVolumeToFill();
  Serial.print("Fill Volume: ");
  Serial.println(volumeToFill);

  if (startFillPressed && volumeToFill)
  {
    Serial.println("Start Fill Button Pressed !!");
    startFilling();
  }
}

void startFilling()
{
  pulseCount = 0;
  filledVolume = 0.0;
  totalMilliLitres = 0.0;

  digitalWrite(pumpRelay, LOW); // Turn ON the relay
  Serial.println("Pump ON");
  attachInterrupt(digitalPinToInterrupt(waterFlowSensorPin), pulseCounter, RISING);
  startTime = millis();

  while (filledVolume < volumeToFill)
  {
    if (millis() > (startTime + 1000))
    {
      filledVolume = calculateFilledVolume();
      Serial.print("Filled Volume: ");
      Serial.println(filledVolume);

      if (filledVolume >= volumeToFill)
      {
        Serial.print("Bottle filled !! - ");
        Serial.println(filledVolume);

        digitalWrite(pumpRelay, HIGH); // Turn OFF the pump
        Serial.println("Pump OFF");

        pulseCount = 0;
        detachInterrupt(digitalPinToInterrupt(waterFlowSensorPin));
        startFillPressed = false;
        delay(500);
        filledVolume = 0.0;
        delay(500);

        totalMilliLitres = 0.0;
        delay(500);

        break;
      }
    }
  }
}

long calculateFilledVolume()
{
  detachInterrupt(digitalPinToInterrupt(waterFlowSensorPin));
  flowRate = ((100.0 / (millis() - startTime)) * pulseCount) / calibrationFactor;

  flowMilliLitres = (flowRate / 60) * 1000;

  totalMilliLitres += flowMilliLitres;

  pulseCount = 0;
  startTime = millis();
  attachInterrupt(digitalPinToInterrupt(waterFlowSensorPin), pulseCounter, FALLING);

  return totalMilliLitres;
}

void pulseCounter()
{
  pulseCount++;
}

void startFillInt()
{
  Serial.println("Start Int");
  startFillPressed = true;
}

void getVolumeToFill()
{
  if (digitalRead(_250) == LOW)
  {
    volumeToFill = 250;
  }
  else if (digitalRead(_500) == LOW)
  {
    volumeToFill = 500;
  }
  else if (digitalRead(_750) == LOW)
  {
    volumeToFill = 750;
  }
  else if (digitalRead(_1000) == LOW)
  {
    volumeToFill = 1000;
  }
  else if (digitalRead(_2000) == LOW)
  {
    volumeToFill = 2000;
  }
  else if (digitalRead(_2500) == LOW)
  {
    volumeToFill = 2500;
  }
  else if (digitalRead(_5000) == LOW)
  {
    volumeToFill = 5000;
  }
}
