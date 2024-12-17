#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define GREEN_LED 4 
#define RED_LED 5   
#define MQ3_PIN A0  

unsigned long lastReadingTime = 0;
const unsigned long readingInterval = 30000; 

void setup() {
    Serial.begin(19200);
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    lcd.init();
    lcd.backlight();
    lcd.setCursor(3, 0);
    lcd.print("Initializing...");
}

void loop() {
    unsigned long currentMillis = millis();

    if (currentMillis - lastReadingTime >= readingInterval) {
        int alcoholValue = analogRead(MQ3_PIN);

        Serial.println(alcoholValue);

        lcd.clear();
        lcd.setCursor(2, 1);
        lcd.print("Alcohol Value:");
        lcd.setCursor(2, 2);
        lcd.print(alcoholValue);

        if (alcoholValue < 400) {
            digitalWrite(GREEN_LED, HIGH);
            digitalWrite(RED_LED, LOW);
            lcd.setCursor(2, 3);
            lcd.print("You can drive!");
        } else {
            digitalWrite(GREEN_LED, LOW);
            digitalWrite(RED_LED, HIGH);
            lcd.setCursor(2, 3);
            lcd.print("Cannot drive!");
        }

        lastReadingTime = currentMillis; 
    }
}
