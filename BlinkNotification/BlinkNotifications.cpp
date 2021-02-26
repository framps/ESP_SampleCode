#include "BlinkNotifications.h"

namespace framp {

  BlinkNotifications::BlinkNotifications (uint8_t gpio, int blinkPeriod) : 
    loopEndless(false), 
    gpio(gpio), 
    blinkPeriod(blinkPeriod) {
  }
  
  void BlinkNotifications::loop() {
  
    if ( this->repeatCount > 0 || this->repeatCount == -1 ) {    
      
      // handle on state of LED
      if ( this->blinkOn && this->onTimer > 0 ) {
        if ( ! this->LEDStateOn ) {
          digitalWrite(this->gpio, LOW);
          this->LEDStateOn = true;
        }
        if ( millis() - this->onTimer < this->onTime ) { // still not expired
          return;
        }
        this->offTimer = millis();                     // flip to off state
        this->blinkOn = false;
        return;
      }
      
      // handle off state
      else {
        if ( this->LEDStateOn ) {
          digitalWrite(this->gpio, HIGH);
          this->LEDStateOn = false;
        }
        if ( millis() - this->offTimer < this->offTime ) { // still not expired
          return;
        }
        this->onTimer = millis();                     // flip to on state
        this->blinkOn = true;
      }
  
      // move on to next char
      this->blinkOffset++;

      // terminate repeats if requested
      if ( this->blinkOffset > this->blinkTemplate.length() - 1 ) { // char sequence processed, move on to next repeat
        if ( ! this-> loopEndless ) {
          this->repeatCount--;
        }          
        this->blinkOffset = 0;
      }
  
      this->setBlinkTimes(this->blinkTemplate[this->blinkOffset]);
  
    }
  }
  
  void BlinkNotifications::setBlinkTimes(char c) {
    switch (c) {
      case '-':
        this->onTime = this->blinkPeriod * 3 / 2;
        this->offTime = this->blinkPeriod / 3;
        break;
      case '.':
        this->onTime = this->blinkPeriod / 2;
        this->offTime = this->blinkPeriod / 2;
        break;
      case ' ':
        this->onTime = 0;
        this->offTime = this->blinkPeriod;
        break;
    }
  }

  void BlinkNotifications::setup () {
    pinMode(this->gpio, OUTPUT);
    digitalWrite(this->gpio, HIGH);       // make sure LED is off
    this->LEDStateOn = false;
  }    
   
  void BlinkNotifications::start (string blinkTemplate, unsigned repeat) {

    this->loopEndless = repeat == -1;
    this->repeatCount = repeat;
    
    this->blinkTemplate = blinkTemplate;
    this->blinkOffset = 0;
  
    this->onTimer = millis();
    this->blinkOn = true;
    this->setBlinkTimes(this->blinkTemplate[this->blinkOffset]);
  
    digitalWrite(this->gpio, HIGH);       // make sure LED is off
    this->LEDStateOn = false;
  
    this->loop();
  }

  void BlinkNotifications::stop () {
    this->repeatCount = 1;                // finish current loop
    this->loopEndless=false;              // disable endless loop
    digitalWrite(this->gpio, HIGH);       // make sure LED is off
    this->LEDStateOn = false;
  }

  void BlinkNotifications::stopForce () {
    this->repeatCount = 0;                // just stop 
    digitalWrite(this->gpio, HIGH);       // make sure LED is off
    this->LEDStateOn = false;
  }

  void BlinkNotifications::setPeriod( int blinkPeriod ) {
    this->blinkPeriod = blinkPeriod;
  }    

  void BlinkNotifications::setTemplate( string blinkTemplate ) {
    this->blinkTemplate= blinkTemplate;
  }

}
