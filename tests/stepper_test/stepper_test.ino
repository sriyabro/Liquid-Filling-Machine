/* Example sketch to control a stepper motor with TB6600 stepper motor driver and Arduino without a library: continuous rotation. More info: https://www.makerguides.com */

// Define stepper motor connections:
#define stepperPulse 11
#define stepperDir 12

int upButton = A2;
int downButton = A3;

void setup()
{
  pinMode(stepperPulse, OUTPUT);
  pinMode(stepperDir, OUTPUT);
  pinMode(upButton, INPUT_PULLUP);
  pinMode(downButton, INPUT_PULLUP);

  // Set the spinning direction CW/CCW:
  digitalWrite(stepperDir, HIGH);
}

void loop()
{
  // These four lines result in 1 step:

  while (digitalRead(downButton) == LOW || digitalRead(upButton) == LOW)
  {
    if (digitalRead(downButton) == LOW)
    {
      digitalWrite(stepperDir, LOW);
    }
    else {
      digitalWrite(stepperDir, HIGH);
    }
    digitalWrite(stepperPulse, HIGH);
    delayMicroseconds(150);
    digitalWrite(stepperPulse, LOW);
    delayMicroseconds(150);
  }
}