#include <Arduino.h>
#include "RotaryEncoderIRQ.h"
/***
 * 03.21/AGS
 * Dev platform for interupt driven Rotary Encoder 
 * from https://www.tutorialspoint.com/arduino/arduino_interrupts.htm
 *
 * Pin interrupt https://www.electrosoftcloud.com/en/pcint-interrupts-on-arduino/
 * 
 *          *       *       *       *
 * B0   __----____----____----____----
 * B1   ____----____----____----____--
 *        ' ' ' ' ' ' ' ' ' ' ' ' ' '
 * 
 *   * : resting state
 *   ' : interrupt on B0
 * 
 *   
 * CW  :  1-0-2-3
 * CCW :  2-0-1-3
 * 
 * 
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

  //Serial.print("Port mask = ");
  //Serial.println(_pcicrPortMask);

  //initialize the 
  _dtPin = dtPin;
  _clkPin = clkPin;
  _btnPin = btnPin;
  _debounceDelayMilliSec = debounceDelayMilliSec;

  //calculate pin masks 
  _clkPinMask = 1 << clkPin;
  _dtPinMask = 1 << dtPin;
  _btnPinMask = 1 << btnPin;
  //_pinsMask = _dtPinMask | _clkPinMask | _btnPinMask;   //interrupt on all pins transient
  //initialize encoder state
  _event = ROTARY_ENCODER_EVENT_NONE;
  _rotaryEncoderStateSum = _oldRotaryEncoderState = _newRotaryEncoderState = 0;
  _btnCntLow = _btnCntHigh = _btnState = 0; 
};

void RotaryEncoderIRQ::Init(){
  
  //if pins are not correctly connected then do nothing
  if(_pcicrPortMask == PCICR_PORT_MASK_NONE) return;

  //Serial.println("Init");
  pinMode(_dtPin,INPUT);
  pinMode(_clkPin,INPUT);
  pinMode(_btnPin,INPUT);

  //enable interrupts on the pins port 
  PCICR  |= _pcicrPortMask;  

  //set the pins that cause interrupt
  if(_pcicrPortMask == PCICR_PORT_MASK_PORTB){
    //Serial.println("IR on port B");
    PCMSK0 |= _dtPinMask | _clkPinMask | _btnPinMask; 
  }else if (_pcicrPortMask == PCICR_PORT_MASK_PORTD){
    //Serial.println("IR on port D");
    PCMSK2 |= _dtPinMask | _clkPinMask | _btnPinMask;   
  }
}



void RotaryEncoderIRQ::HandleIRQ(){
  int inputByte = 0;
  //Serial.println("interrupt");
  if(_event != ROTARY_ENCODER_EVENT_NONE) return;  //last event has not been read - ignore interrupt


  //wait a little for the contacts to debounce
  delayMicroseconds(_debounceDelayMilliSec);
  inputByte = (_pcicrPortMask == PCICR_PORT_MASK_PORTB) ? PINB : PIND;
  //Serial.println(inputByte);

  //read clk and dt pins so that B0 = clk & B1 = dt 
  _newRotaryEncoderState = (inputByte & _dtPinMask) >> (_dtPin-1) | (inputByte & _clkPinMask) >> _clkPin;
  //Serial.print("roState = ");
  //Serial.println(_newRotaryEncoderState);

  if(_newRotaryEncoderState != _oldRotaryEncoderState){
    //Serial.println( nState);
    _rotaryEncoderStateSum += _newRotaryEncoderState;
    if(_newRotaryEncoderState == 3 ){
      if(_rotaryEncoderStateSum == 6){
        _event = (_oldRotaryEncoderState == 1)? ROTARY_ENCODER_EVENT_CW : ROTARY_ENCODER_EVENT_CCW;
      };
      _rotaryEncoderStateSum = 0;
    }
  };
  _oldRotaryEncoderState = _newRotaryEncoderState;
}


ROTARY_ENCODER_EVENT_TYPE RotaryEncoderIRQ::GetEvent(){
  ROTARY_ENCODER_EVENT_TYPE e = _event;
  _event = ROTARY_ENCODER_EVENT_NONE;
  return e;
}


