#pragma once

// -------------------------------------------------------------------------------------------------------------
// Simple class to create LED blink notifications with a blink pattern string which can include "-", "." and " "
// -------------------------------------------------------------------------------------------------------------
//
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

#include <Arduino.h>
#include <Ticker.h>

namespace framp {

	class BlinkNotification {

  public:
    BlinkNotification(
      uint8_t gpio,
      unsigned blinkPeriod = 1000,             // period in ms
      std::string blinkTemplate = ".",
      unsigned repeatCount = 0,                // just execute blink loop once
      unsigned delayTime = 1000);              // delay at the end of blink loop in ms

    ~BlinkNotification() {
      this->stop();
      };

    void start();           // start blink loop
    void stop();            // stop blink loop immediately

    // setters and getters
    void setRepeatCount(unsigned repeatCount) { this->repeatCount = repeatCount; };
    unsigned getRepeatCount() { return this->repeatCount; };
    void setDelayTime(unsigned delayTime) { this->delayTime = delayTime; };
    unsigned getDelayTime() { return this->delayTime; };
		// tests
    bool isActive() { return this->active; };

  // private: - not private because of C++ Lambda issue
    void flipLED();              // flip LED status

  private:
	  uint8_t gpio;			     // gpio of LED
		int blinkPeriod;       // period of a blink in ms

    Ticker ticker;         // ticker used to flip LED state
    unsigned onTime;       // time to have LED on for char
    unsigned offTime;      // time to have LED off for char
    unsigned gapTime;      // time to have a gap between chars
    unsigned delayTime;    // delay at end of blink loop in ms

    bool LEDStateOn;       // LED on or off state
    bool loopEndless;      // blink all the time
    bool active;           // blink loop isrunning

    unsigned repeatCount;  // number of blink repeats
    std::string blinkPattern;  // blink chars
    int blinkOffset;       // current processed char in blinkPattern

  private:
    void setBlinkTimes(char c);  // calculate on/off delays
    void turnOn();
    void turnOff();
	};

}
