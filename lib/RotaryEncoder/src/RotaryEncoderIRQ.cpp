#include <Arduino.h>
#include "RotaryEncoderIRQ.h"
/***
 * (c)2022 Arni Geir Sigurdsson (arni.geir.sigurdsson@gmail.com) 
 ***/
enum  cpicr_interrupt_port{
                PCICR_PORT_MASK_NONE = 0,
                PCICR_PORT_MASK_PORTB = 1,
                PCICR_PORT_MASK_PORTD = 4,
              } ;

RotaryEncoderIRQ::RotaryEncoderIRQ(int clkPin,int dtPin,int btnPin,int debounceDelayMilliSec){

  //calculate the the port (based on clock pin)
  _pcicrPortMask = PCICR_PORT_MASK_NONE;

  if( (clkPin>=0 && clkPin <=7) && (dtPin>=0 && dtPin <=7) && btnPin>=0 && btnPin <=7) _pcicrPortMask  = PCICR_PORT_MASK_PORTD;
  else if( (clkPin > 7 && clkPin <=13) && (dtPin > 7 && dtPin <=13) && (btnPin > 7 && btnPin <=13)) _pcicrPortMask = PCICR_PORT_MASK_PORTB;

  //initialize the 
  _dtPin = dtPin;
  _clkPin = clkPin;
  _btnPin = btnPin;
  _debounceDelayMilliSec = debounceDelayMilliSec;

  //calculate pin masks 
  _clkPinMask = 1 << clkPin;
  _dtPinMask = 1 << dtPin;
  _btnPinMask = 1 << btnPin;
  //initialize encoder state
  _event = ROTARY_ENCODER_EVENT_NONE;
  _rotaryEncoderStateSum = _oldRotaryEncoderState = _newRotaryEncoderState = 0;
  _oldButtonState =  0; 
};

void RotaryEncoderIRQ::Init(){
  
  //if pins are not correctly connected then do nothing
  if(_pcicrPortMask == PCICR_PORT_MASK_NONE) return;

  pinMode(_dtPin,INPUT);
  pinMode(_clkPin,INPUT);
  pinMode(_btnPin,INPUT);

  //enable interrupts on the pins port 
  PCICR  |= _pcicrPortMask;  

  //set the pins that cause interrupt
  if(_pcicrPortMask == PCICR_PORT_MASK_PORTB){
    PCMSK0 |= _dtPinMask | _clkPinMask | _btnPinMask; 
  }else if (_pcicrPortMask == PCICR_PORT_MASK_PORTD){
    PCMSK2 |= _dtPinMask | _clkPinMask | _btnPinMask;   
  }
}



void RotaryEncoderIRQ::HandleIRQ(){

  int inputByte = 0;
  int buttonPressed = 0;
  if(_event != ROTARY_ENCODER_EVENT_NONE){
     return;  //last event has not been read - ignore interrupt
  }

  //wait a little for the contacts to debounce
  delayMicroseconds(_debounceDelayMilliSec);
  inputByte = (_pcicrPortMask == PCICR_PORT_MASK_PORTB) ? PINB : PIND;


  // -------- handle rotary encoder state changes

  //read clk and dt pins so that B0 = clk & B1 = dt 
  _newRotaryEncoderState = (inputByte & _dtPinMask) >> (_dtPin-1) | (inputByte & _clkPinMask) >> _clkPin;
  buttonPressed = ! ( (inputByte & _btnPinMask)  >> _btnPin);

  if(_newRotaryEncoderState != _oldRotaryEncoderState){
    _rotaryEncoderStateSum += _newRotaryEncoderState;
    if(_newRotaryEncoderState == 3 ){
      if(_rotaryEncoderStateSum == 6){
        _event = (_oldRotaryEncoderState == 1)? ROTARY_ENCODER_EVENT_CW : ROTARY_ENCODER_EVENT_CCW;
      };
      _rotaryEncoderStateSum = 0;
    }
  };
  _oldRotaryEncoderState = _newRotaryEncoderState;

  // handle button state changes 
  if(buttonPressed) {
    _event = ROTARY_ENCODER_EVENT_BUTTON;
    _oldButtonState = buttonPressed;
  }else{
    _oldButtonState = 0;
  }
}


ROTARY_ENCODER_EVENT_TYPE RotaryEncoderIRQ::GetEvent(){
  ROTARY_ENCODER_EVENT_TYPE e = _event;
  _event = ROTARY_ENCODER_EVENT_NONE;
  return e;
}



