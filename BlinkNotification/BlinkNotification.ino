#include "BlinkNotification.h"

using namespace framp;

Ticker createError;                                 // ticker to simulate an error situation
bool errorOccured = false;
void setError() {
  errorOccured = true;
}

BlinkNotification working(BUILTIN_LED,100);        // active blink pattern to signal ESP is active working
BlinkNotification sos(BUILTIN_LED,500);            // error blink pattern to signal an error condition
 
void setup() {
  
  Serial.begin(115200);
  Serial.println();
  
  sos.setup();

  working.setup();
  working.start(". ",-1);                           // -1 will blink the pattern forever
  Serial.println("Working ...");

  createError.once_ms(5000, setError);              // activate error simulation after 5 seconds

}

void loop() {  

  if ( errorOccured ) {
    working.stopForce();                            // terminate working blink pattern
    errorOccured = false;   
    Serial.println("SOS blinks...");              
    sos.start("... --- ...   ",-1);                 // blink the error pattern (SOS)
  }
  
}
