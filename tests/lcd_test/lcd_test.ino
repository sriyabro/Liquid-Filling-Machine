#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

byte q11[] = {
	B11111,
	B11111,
	B11000,
	B11000,
	B11001,
	B11001,
	B11001,
	B11001};

byte q12[] = {
	B11111,
	B11111,
	B00000,
	B00000,
	B11111,
	B11111,
	B10000,
	B10000};

byte q13[] = {
	B11111,
	B11111,
	B00011,
	B00011,
	B10011,
	B10011,
	B00011,
	B00011};

byte q21[] = {
	B11001,
	B11001,
	B11001,
	B11001,
	B11000,
	B11000,
	B11111,
	B11111};

byte q22[] = {
	B10000,
	B10000,
	B11111,
	B11111,
	B00000,
	B00000,
	B11111,
	B11111};

byte q23[] = {
	B00011,
	B00011,
	B10011,
	B10011,
	B00011,
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

void setup()
{
	lcd.begin();
	printCustomChar(0, q11, 1, 1);
	printCustomChar(2, q12, 1, 2);
	printCustomChar(3, q13, 1, 3);
	printCustomChar(4, q21, 2, 1);
	printCustomChar(5, q22, 2, 2);
	printCustomChar(6, q23, 2, 3);
	printCustomChar(7, q24, 2, 4);
}

void loop() {}

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
