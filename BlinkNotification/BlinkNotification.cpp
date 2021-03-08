// -------------------------------------------------------------------------------------------------------------
// Simple class to create LED blink notifications with a blink pattern string which can include "-", "." and " "
// -------------------------------------------------------------------------------------------------------------

// Thank you __deets__ for your valuable help and feedback 

/*
#######################################################################################################################
#
#    Copyright (c) 2021 framp at linux-tips-and-tricks dot de
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#######################################################################################################################
*/

#include "BlinkNotification.h"

namespace framp {

BlinkNotification::BlinkNotification(uint8_t gpio, unsigned blinkPeriod, std::string blinkPattern, unsigned repeatCount, unsigned delayTime ) :
  gpio(gpio),
  repeatCount(repeatCount),
  blinkPattern(blinkPattern),
  blinkPeriod(blinkPeriod),
  delayTime(delayTime),
  active(false),
  blinkOffset(0) {
    pinMode(this->gpio, OUTPUT);
    digitalWrite(this->gpio, HIGH);       // make sure LED is off
    this->LEDStateOn = false;
}

void BlinkNotification::flipLED() {

  if ( this->repeatCount > 0 || this->loopEndless ) { 

    // handle on state

    if ( this->LEDStateOn && this->onTime > 0) {
      digitalWrite(this->gpio, LOW);
      this->LEDStateOn = false;
      this->ticker.once_ms(this->onTime, [this]() { this-> flipLED(); });
      return;
    }

    // handle off state
    else {
      digitalWrite(this->gpio, HIGH);
      this->LEDStateOn = true;
    }

    // move on to next char
    this->blinkOffset++;

    int loopEndDelay=0;
    
    // terminate repeats if requested
    if ( this->blinkOffset > this->blinkPattern.length() - 1 ) { // char sequence processed, move on to next repeat
      loopEndDelay = this->delayTime;
      if ( ! this-> loopEndless ) {
        if (--this->repeatCount <= 0 ) { 
          this->stop();
          return;                       // terminate LED flip flop if number of repeats executed
        }
      }
      this->blinkOffset = 0;            // start with first char again and repeat to blink pattern 
    }

    this->setBlinkTimes(this->blinkPattern[this->blinkOffset]);
    this->ticker.once_ms(this->offTime + loopEndDelay, [this]() { this-> flipLED(); });

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

void BlinkNotification::start () {

  this->setBlinkTimes(this->blinkPattern[this->blinkOffset]);
  this->loopEndless = this-> repeatCount == -1;
  if ( this->loopEndless ) {
    this->repeatCount = 1;
  }

  digitalWrite(this->gpio, HIGH);       // make sure LED is off
  this->LEDStateOn = true;
  this->active = true;
  this->flipLED();
}

void BlinkNotification::stop() {
  this->ticker.detach();
  digitalWrite(this->gpio, HIGH);       // make sure LED is off
  this->LEDStateOn = false;
  this->active = false;
}

}
