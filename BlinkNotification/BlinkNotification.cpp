// -------------------------------------------------------------------------------------------------------------
// Simple class to create LED blink notifications with a blink pattern string which can include "-", "." and " "
// -------------------------------------------------------------------------------------------------------------

// Thank you __deets__ for your valuable help and feedback 

/*
#######################################################################################################################
#
#    Copyright (c) 2021,2022 framp at linux-tips-and-tricks dot de
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
  gapTime(delayTime/10),
  active(false),
  blinkOffset(0) {
  pinMode(this->gpio, OUTPUT);
  this->turnOff();
}

void call( BlinkNotification *obj )
{
   obj->flipLED() ;
}

void BlinkNotification::flipLED() {

  if ( this->repeatCount > 0 || this->loopEndless ) { 

    // handle on state

    if ( ! this->LEDStateOn && this->onTime > 0) {
      this->turnOn();
	    this->ticker.once_ms(this->onTime, call, this);
      return;
    }

    // handle on state
    else {
      this->turnOff();
    }

    delay(this->gapTime);

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
    this->ticker.once_ms(this->offTime + loopEndDelay, call, this );
  }
}

void BlinkNotification::setBlinkTimes(char c) {

  switch (c) {
    case '-':
      this->onTime = this->blinkPeriod * 5 / 6;
      break;
    case '.':
      this->onTime = this->blinkPeriod * 1 / 6;
      break;
    case ' ':
      this->onTime = 0;
      break;
  }
  this->offTime = this->blinkPeriod - this->onTime;

}

void BlinkNotification::start () {

  this->blinkOffset = 0;
  this->setBlinkTimes(this->blinkPattern[this->blinkOffset]);
  this->loopEndless = this-> repeatCount == -1;
  if ( this->loopEndless ) {
    this->repeatCount = 1;
  }

  this->turnOff();
  this->active = true;
  this->flipLED();
}

void BlinkNotification::stop() {
  this->ticker.detach();
  this->active = false;
  this->turnOff();
}

void BlinkNotification::turnOff() {
  digitalWrite(this->gpio, LOW);
  this->LEDStateOn = false;
}

void BlinkNotification::turnOn () {
  digitalWrite(this->gpio, HIGH);
  this->LEDStateOn = true;
}

}
