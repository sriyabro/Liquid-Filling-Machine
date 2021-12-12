#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

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

void setup()
{
  lcd.begin();
  lcd.backlight();
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
  printCustomChar(10, n17, 1,9);
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
  delay(1000);
}

void loop()
{
  
}

void printCustomChar(uint8_t location, uint8_t charmap[], int row, int col)
{
  lcd.createChar(location, charmap);
  lcd.setCursor(col - 1, row - 1);
  lcd.write(location);
}

void printToLCD(String text, int lineNo)
{
  lcd.setCursor(0, lineNo - 1);
  lcd.print("                "); // clear line before print
  lcd.setCursor(0, lineNo - 1);
  delay(200);
  lcd.print(text);
}