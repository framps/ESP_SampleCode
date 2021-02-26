#ifndef FRAMP_BLINKNOTIFICATION_H
#define FRAMP_BLINKNOTIFICATION_H

#include <Arduino.h>
#include <Ticker.h>

using namespace std;

namespace framp {

	class BlinkNotifications {

    private:  
    
  	  uint8_t gpio;			     // gpio of LED
  		int blinkPeriod;       // period of a blink in ms
  
      int onTime;            // time to have LED on for char
      int onTimer;           // time to have LED off for char
      int offTime;           // timer to count on time
      int offTimer;          // timer to count off time
      
      bool blinkOn;          // blink state
      bool LEDStateOn;       // LED on or off state
      bool loopEndless;      // blink all the time
      
      unsigned repeatCount;  // number of blink repeats
      string blinkTemplate;  // chars to blink
      int blinkOffset;       // current processed char in blinkTemplate
  		 
    public:
    
      BlinkNotifications(uint8_t gpio  = BUILTIN_LED, int blinkPeriod = 1000); // period in ms
      ~BlinkNotifications() {};
      void setup();          // call in setup()
      void loop();           // call in loop()
      void start(string blinkTemplate, unsigned repeat = 3);  // repeat of -1 is endless repeat
      void stopForce();      // stop blink immediately
      void stop();           // stop blink but wait for blink sequence to finish
      void setPeriod( int blinkPeriod );
      void setTemplate( string blinkTemplate );
       
    private:
    
      void setBlinkTimes(char c);  // calculate on/off delays
	};
  
}

#endif
