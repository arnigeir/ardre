#ifndef ROTARY_ENCODER_IRQ_H
#define ROTARY_ENCODER_IRQ_H

/*
(c)Arni Geir Sigurdsson (arni.geir.sigurdsson@gmail.com) 2022

Rotary encoder interrupt driver

Uses 34b RAM and about 500bb FLASH

EXAMPLE
===========

// Enable interrupts for port D
// dt pin 4
// clk pin 5
// sw pin 6
// debounce delay is 5mSec

#define dtPin 4
#define clkPin 5
#define swPin 6
#define reDebounceDelay_mSec 5

RotaryEncoderIRQ rotaryEncoderIRQ = RotaryEncoderIRQ(RotaryEncoderIRQ::IRQ_PORTD,dtPin,clkPin,swPin,reDebounceDelay_mSec);

void setup() {
  Serial.begin(9600);
  rotaryEncoderIRQ.Init();
}

  
ISR(PCINT2_vect){
  rotaryEncoderIRQ.HandleIRQ();
}


void loop() {
  int event = rotaryEncoderIRQ.GetEvent();
  if(event ){
    if(event == RotaryEncoderIRQ::ROTARYENCODER_CW) Serial.println("CW");
    else if (event == RotaryEncoderIRQ::ROTARYENCODER_CCW)  Serial.println("CCW");
    event = 0;
  }
  delay(100);

*/


typedef enum  {
                ROTARY_ENCODER_PORTB = 1,
                ROTARY_ENCODER_PORTD = 4,
              } ROTARY_ENCODER_PORT_TYPE;
typedef enum {
              ROTARY_ENCODER_EVENT_NONE=0,
              ROTARY_ENCODER_EVENT_SWITCH,
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
    //void ResetEvent();   //call this after event has been consumed and IRQ routine buffer cleared for next event

    //disable copy constructor and assignment constructor ...
    //RotaryEncoderIRQ(const RotaryEncoderIRQ&)  =  delete;  
    //RotaryEncoderIRQ& operator= (const RotaryEncoderIRQ&) = delete;
                                               
 private:
   RotaryEncoderIRQ();  //no default constructor

   int _pcicrPortMask;

   int _btnState,_btnPin,_btnPinMask,_btnCntLow,_btnCntHigh;
   ROTARY_ENCODER_PORT_TYPE _irqPort;

   int _rotaryEncoderStateSum;
   int _oldRotaryEncoderState,_newRotaryEncoderState;
   int _dtPin,_clkPin;
   int _dtPinMask,_clkPinMask,_pinsMask;
   int _debounceDelayMilliSec;
   volatile ROTARY_ENCODER_EVENT_TYPE _event;
   int *inputPort;
};

#endif