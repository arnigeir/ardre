#ifndef ROTARY_ENCODER_IRQ_H
#define ROTARY_ENCODER_IRQ_H

/*
(c)2022 Arni Geir Sigurdsson (arni.geir.sigurdsson@gmail.com) 

Rotary encoder interrupt driver

Uses 34 bytes of RAM and about 500 bytes of FLASH memory

Rotary encoder signals:

  CLK     __---------________---------________-------.........
  DT     _______---------________---------______-----.........
  BUTTOM -----------------------------------------_______-----                                   
  STATE   0 | 1 | 3  | 2 | 0 | 1 | 3  | 2 | 0 | 3 |
  INT       *   *    *   *   *   *    *   *   *   *      *   

  EVENT:         CW               CW           CW  BUTTON
State changes and direction 

          States       
  CCW  :  1-0-2-3
  CW   :  2-0-1-3

A change in encoder state occurs when State=3 and accumulated sum of states since last State=3 is 6
ie 1-0-2-3 ; current state is 3,previous states sums to 6 and last state is 2 => CCW

Example use
===========

// NOTE : All 3 pins must be on either B or D port
// dt pin 4
// clk pin 5
// sw pin 6
// debounce delay is 5mSec

#define dtPin 4
#define clkPin 5
#define buttonPin 6
#define debounceDelay_mSec 5

RotaryEncoderIRQ *rotaryEncoderIRQ=nullptr;

void setup() {
  Serial.begin(9600);
  RotaryEncoderIRQ re = RotaryEncoderIRQ(clkPin,dtPin,buttonPin,reDebounceDelay_mSec); 
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
  else if (event == ROTARY_ENCODER_EVENT_BUTTON) Serial.println("CLICK");

  delay(100);

Info sources :
  Dev platform for interupt driven Rotary Encoder from https://www.tutorialspoint.com/arduino/arduino_interrupts.htm
  Pin interrupt https://www.electrosoftcloud.com/en/pcint-interrupts-on-arduino/

*/



typedef enum {
              ROTARY_ENCODER_EVENT_NONE=0,
              ROTARY_ENCODER_EVENT_BUTTON,
              ROTARY_ENCODER_EVENT_CW,
              ROTARY_ENCODER_EVENT_CCW
              } ROTARY_ENCODER_EVENT_TYPE;

class RotaryEncoderIRQ{
  public:
    //enum Event {ROTARYENCODER_NONE=0,ROTARYENCODER_CW,ROTARYENCODER_CCW};
    //enum IRQ_PORT {IRQ_PORTB = 1,IRQ_PORTC = 2, IRQ_PORTD = 4};
    //all rotary encoder pins must map to either port B (pin D8-D13) or port D (pin D0-D7)

    RotaryEncoderIRQ(int clkPin,int dtPin,int swPin,int debounceDelayMilliSec);
    void Init();                                                            //call init in the setup function
    void HandleIRQ();                                                       //call this function from the interrupt routine
    ROTARY_ENCODER_EVENT_TYPE GetEvent();      //call this from main loop to get last buffered encoder value

    //disable copy constructor and assignment constructor ...
    // RotaryEncoderIRQ(const RotaryEncoderIRQ&)  =  delete;  
    // RotaryEncoderIRQ& operator= (const RotaryEncoderIRQ&) = delete;

                                               
 private:
   RotaryEncoderIRQ();  //no default constructor

   //ROTARY_ENCODER_PORT_TYPE _irqPort;
   volatile ROTARY_ENCODER_EVENT_TYPE _event;

   int _oldRotaryEncoderState;
   int _newRotaryEncoderState;
   int _rotaryEncoderStateSum;
   //int _newButtonState;
   int _oldButtonState;
   int _dtPin;
   int _clkPin;
   int _btnPin;
   int _dtPinMask;
   int _clkPinMask;
   int _btnPinMask;
   int _pcicrPortMask;
   int _debounceDelayMilliSec;
};

#endif