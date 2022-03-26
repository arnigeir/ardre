#include <Arduino.h>
#include "RotaryEncoderIRQ.h"

#define clkPIN 4
#define dtPIN 5 
#define btnPIN 6
#define bounceDelay 5



RotaryEncoderIRQ *rotaryEncoderIRQ; // = RotaryEncoderIRQ(clkPIN,dtPIN,btnPIN,bounceDelay);

void setup() {
  Serial.begin(9600);
  RotaryEncoderIRQ re = RotaryEncoderIRQ(clkPIN,dtPIN,btnPIN,bounceDelay);
  rotaryEncoderIRQ = &re;
  rotaryEncoderIRQ->Init();
}


ISR(PCINT2_vect){
  rotaryEncoderIRQ->HandleIRQ();
}



void loop() {
  
  ROTARY_ENCODER_EVENT_TYPE event = rotaryEncoderIRQ->GetEvent();
  
  if(event == ROTARY_ENCODER_EVENT_CW) Serial.println("CW");
  else if (event == ROTARY_ENCODER_EVENT_CCW)  Serial.println("CCW");

  delay(100);
  

}