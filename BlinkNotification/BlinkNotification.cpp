#include "BlinkNotification.h"

namespace framp {

BlinkNotification::BlinkNotification(uint8_t gpio, int blinkPeriod) :
  loopEndless(false),
  gpio(gpio),
  blinkPeriod(blinkPeriod) {
}

void BlinkNotification::flipLED() {

  if ( this->repeatCount > 0 || this->loopEndless ) { 

    // handle on state

    if ( this->LEDStateOn && this->onTime > 0) {
      digitalWrite(this->gpio, LOW);
      this->LEDStateOn = false;
      this->ticker.once_ms(this->onTime, std::bind(&BlinkNotification::flipLED, this));
      return;
    }

    // handle off state
    else {
      digitalWrite(this->gpio, HIGH);
      this->LEDStateOn = true;
    }

    // move on to next char
    this->blinkOffset++;

    // terminate repeats if requested
    if ( this->blinkOffset > this->blinkPattern.length() - 1 ) { // char sequence processed, move on to next repeat
      if ( ! this-> loopEndless ) {
        if (--this->repeatCount <= 0 ) { 
          return;                       // terminate LED flip flop if number of repeats executed
        }
      }
      this->blinkOffset = 0;            // start with first char again and repeat to blink pattern 
    }

    this->setBlinkTimes(this->blinkPattern[this->blinkOffset]);
    this->ticker.once_ms(this->offTime, std::bind(&BlinkNotification::flipLED, this));

  }
}

void BlinkNotification::setBlinkTimes(char c) {

  switch (c) {
    case '-':
      this->onTime = this->blinkPeriod * 3 / 4;
      break;
    case '.':
      this->onTime = this->blinkPeriod * 1 / 4;
      break;
    case ' ':
      this->onTime = 0;
      break;
  }
  this->offTime = this->blinkPeriod - this->onTime;

}

void BlinkNotification::setup () {
  pinMode(this->gpio, OUTPUT);
  digitalWrite(this->gpio, HIGH);       // make sure LED is off
  this->LEDStateOn = false;
}

void BlinkNotification::start (std::string blinkPattern, unsigned repeat) {

  this->loopEndless = repeat == -1;
  this->repeatCount = repeat;

  this->blinkPattern = blinkPattern;
  this->blinkOffset = 0;

  this->setBlinkTimes(this->blinkPattern[this->blinkOffset]);

  digitalWrite(this->gpio, HIGH);       // make sure LED is off
  this->LEDStateOn = true;

  this->flipLED();

}

void BlinkNotification::stop () {
  this->repeatCount = 1;                // finish current loop
  this->loopEndless = false;            // disable endless loop
  digitalWrite(this->gpio, HIGH);       // make sure LED is off
  this->LEDStateOn = false;
}

void BlinkNotification::stopForce () {
  this->ticker.detach();
  digitalWrite(this->gpio, HIGH);       // make sure LED is off
  this->LEDStateOn = false;
}

}
