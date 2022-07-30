/**
 * @file liquid-filler.ino
 * @author Sriyanjith Herath (sriyabro@gmail.com)
 * @brief 16x2 i2c lcd with 4x4 keypad menu. Inspired by EventSerialKeypad.pde from Alexander Brevig (alexanderbrevig@gmail.com)
x * @version 1.0
 * @date 2022-07-28
 *
 * @copyright  Copyright 2021 Sriyanjith Herath. All rights reserved. Unauthorized access, copying, publishing, sharing, reuse of algorithms, concepts, design patterns and code level demonstrations are strictly prohibited without any written approval of the author.
 */

String menuItems[] = {"250 ml", "500 ml", "1 L", "2 L", "2.5 L", "5 L", "5.5 L"};

// Navigation button variables
int readKey;
int key;
// Menu control variables
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;
int result = 0;
// Creates 3 custom characters for the menu display
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

#include <Wire.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

const byte ROWS = 4; // four rows
const byte COLS = 4; // four columns
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[ROWS] = {4, 5, 6, 7};   // connect to the row pinouts of the keypad
byte colPins[COLS] = {8, 9, 10, 11}; // connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
LiquidCrystal lcd(19, 18, 14, 15, 16, 17); // Parameters: (rs, enable, d4, d5, d6, d7) pin(A5,A4,A3,A2,A1,A0)
byte ledPin = 13;

boolean blink = false;
boolean ledPin_state;

void setup()
{
    lcd.begin(16, 2);
    Serial.begin(9600);
    pinMode(ledPin, OUTPUT);              // Sets the digital pin as output.
    digitalWrite(ledPin, HIGH);           // Turn the LED on.
    ledPin_state = digitalRead(ledPin);   // Store initial LED state. HIGH when LED is on.
    keypad.addEventListener(keypadEvent); // Add an event listener for this keypad
    lcd.createChar(0, menuCursor);
    lcd.createChar(1, upArrow);
    lcd.createChar(2, downArrow);
}

void loop()
{

    mainMenuDraw();
    drawCursor();
    operateMainMenu();
}

// Taking care of some special events.

void keypadEvent(KeypadEvent key)
{
    switch (keypad.getState())
    {
        int readKey;
    case PRESSED:
        break;
    }
}

// This function will generate the 2 menu items that can fit on the screen. They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw()
{
    //  Serial.print(menuPage);
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print(menuItems[menuPage]);
    lcd.setCursor(1, 1);
    lcd.print(menuItems[menuPage + 1]);
    if (menuPage == 0)
    {
        lcd.setCursor(15, 1);
        lcd.write(byte(2));
    }
    else if (menuPage > 0 and menuPage < maxMenuPages)
    {
        lcd.setCursor(15, 1);
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

// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor()
{
    for (int x = 0; x < 2; x++)
    { // Erases current cursor
        lcd.setCursor(0, x);
        lcd.print(" ");
    }

    // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
    // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
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

        switch (button)
        {
        case 1: // When button returns as 0 there is no action taken
            break;
        case 3:
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

            mainMenuDraw();
            drawCursor();
            activeButton = 1;
            break;
        case 4: // This case will execute if the "forward" button is pressed
            button = 0;
            menuItem(cursorPosition);
            activeButton = 1;
            mainMenuDraw();
            drawCursor();
            break;
        case 5:
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
            mainMenuDraw();
            drawCursor();
            activeButton = 1;
            break;
        }
    }
}

// This function is called whenever a button press is evaluated. The LCD shield works by observing a voltage drop across the buttons all hooked up to A0.
int evaluateButton(int x)
{
    int result = 0;
    int key = keypad.getKey();
    if (key < 57)
    {
        delay(100);
    }
    x = key;

    if (x < 51)
    {
        result = 1; // right
    }
    else if (x < 53)
    {
        result = 2; // up
    }
    else if (x < 54)
    {
        result = 3; // down
    }
    else if (x < 55)
    {
        result = 4; // left
    }
    else if (x < 57)
    {
        result = 5; // left
    }

    return result;
}

// If there are common usage instructions on more than 1 of your menu items you can call this function from the sub
// menus to make things a little more simplified. If you don't have common instructions or verbage on multiple menus
// I would just delete this void. You must also delete the drawInstructions()function calls from your sub menu functions.
void drawInstructions()
{
    lcd.setCursor(0, 1); // Set cursor to the bottom line
    lcd.print("Use ");
    lcd.print(byte(1)); // Up arrow
    lcd.print("/");
    lcd.print(byte(2)); // Down arrow
    lcd.print(" buttons");
}

void menuItem(int itemId)
{ // Function executes when you select the 1st item from main menu
    int activeButton = 0;

    lcd.clear();
    lcd.setCursor(3, 0);
    lcd.print(meunItems[itemId]);

    while (activeButton == 0)
    {
        int button;

        button = evaluateButton(readKey);
        switch (button)
        {
        case 2: // This case will execute if the "back" button is pressed
            button = 0;
            activeButton = 1;
            break;
        }
    }
}
