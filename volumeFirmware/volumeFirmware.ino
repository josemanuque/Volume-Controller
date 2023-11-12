#include <LiquidCrystal.h>

int D7_pin = 4;
int D6_pin = 5;
int D5_pin = 6;
int D4_pin =  7;
int EN_pin = 11;
int RS_pin = 12;
int volumeValue = -1;
LiquidCrystal lcd(RS_pin, EN_pin, D4_pin, D5_pin, D6_pin, D7_pin);

void setup(){
  Serial.begin(9600);
  lcd.begin(16, 2);  
}

void loop(){
    int analogInput = analogRead(A0);
    if(volumeValue != analogInput){
       volumeValue = analogInput;
       Serial.println(volumeValue);
    }
    if (Serial.available() > 0) {
    // Lee el byte recibido
      String inputString = Serial.readStringUntil('\n');
      int inputValue = inputString.toInt();

	 // Prints the received value in the LCD
      lcd.clear();
      lcd.print("Volume level is at:");
      lcd.setCursor(0, 1);
      lcd.print(inputString);
    }
    delay(100);
}
