#include "DigiKeyboard.h"

void setup() {
  // Setup Pin as input and set it to low on first run.
  //Serial.begin(9600);
  pinMode(1, INPUT);
  digitalWrite(1, LOW);    
  DigiKeyboard.sendKeyStroke(0); // Necessary to ensure compatibility with older machines
  //Serial.println("Setup was complete");
}

void loop() {
  if(digitalRead(1) == HIGH){
    //Serial.println("Pulsed!");
    DigiKeyboard.delay(500);      // Wait
    DigiKeyboard.sendKeyStroke(44);   // Send space keystroke
    DigiKeyboard.delay(1500);   // Wait
    DigiKeyboard.sendKeyStroke(82);   // Send up arrow again (to raise shade in win10)
    DigiKeyboard.delay(500);  // Wait
    DigiKeyboard.print("5966");  // Enter Password
    DigiKeyboard.delay(500);  // Wait
    DigiKeyboard.sendKeyStroke(KEY_ENTER);  // Send Enter Key
    DigiKeyboard.delay(12000);  // Wait
  }
  DigiKeyboard.delay(200);   // Wait
}
