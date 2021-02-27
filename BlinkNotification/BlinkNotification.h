#pragma once

#include <Arduino.h>
#include <Ticker.h>

/*
 * Simple class to create LED blink notifications with a blink pattern string which can include "-", "." and " "
 */

namespace framp {

	class BlinkNotification {

  public:
    BlinkNotification(uint8_t gpio  = BUILTIN_LED, int blinkPeriod = 1000); // period in ms
    ~BlinkNotification() {};
    void setup();          // call in setup()    
    void start(std::string blinkTemplate, unsigned repeat = 3);  // repeat of -1 is endless repeat
    void stopForce();      // stop blink immediately
    void stop();           // stop blink but wait for blink sequence to finish
  
  private:  
	  uint8_t gpio;			     // gpio of LED
		int blinkPeriod;       // period of a blink in ms

    Ticker ticker;         // ticker used to flip LED state       
    int onTime;            // time to have LED on for char
    int offTime;           // time to have LED off for char
    
    bool LEDStateOn;       // LED on or off state
    bool loopEndless;      // blink all the time
    
    unsigned repeatCount;  // number of blink repeats
    std::string blinkPattern;  // blink chars
    int blinkOffset;       // current processed char in blinkPattern
		    
  private:
    void setBlinkTimes(char c);  // calculate on/off delays
    void flipLED();              // flip LED status
	};
  
}
