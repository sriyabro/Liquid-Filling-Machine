#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup()
{
	// initialize the LCD
	lcd.begin();

	// Turn on the blacklight and print a message.
	// lcd.backlight();
	printToLCD("SELECTED VOLUME", 1);
	printToLCD("250 ml", 2);
	delay(2000);
	printToLCD("SELECTED VOLUME", 1);
	printToLCD("500 ml", 2);
	delay(2000);
	printToLCD("SELECTED VOLUME", 1);
	printToLCD("1000 ml", 2);
	delay(2000);
	printToLCD("SELECTED VOLUME", 1);
	printToLCD("2000 ml", 2);
	delay(2000);
	printToLCD("SELECTED VOLUME", 1);
	printToLCD("2500 ml", 2);
	delay(2000);
	printToLCD("SELECTED VOLUME", 1);
	printToLCD("5000 ml", 2);
	delay(2000);
	printToLCD("START", 1);
	printToLCD(" ", 2);
}

void loop()
{
}

void printToLCD(String text, int lineNo)
{
	lcd.setCursor(0, lineNo - 1);
	lcd.print("                "); // clear line before print
	lcd.setCursor(0, lineNo - 1);
	delay(200);
	lcd.print(text);
}
