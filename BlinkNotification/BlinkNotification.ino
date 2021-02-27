#include "BlinkNotification.h"

using namespace framp;

Ticker createError;             // ticker to simulate an error situation
bool errorOccured = false;
void setError() {
  errorOccured = true;
}

BlinkNotification sos(BUILTIN_LED,500);            // error blink pattern
BlinkNotification working(BUILTIN_LED,100);        // active blink pattern
 
void setup() {
  
  Serial.begin(115200);
  Serial.println();
  
  sos.setup();

  working.setup();
  working.start(". ",-1);
  Serial.println("Working ...");

  createError.once_ms(5000, setError);

}

void loop() {  

  if ( errorOccured ) {
    working.stopForce();
    errorOccured = false;
    Serial.println("SOS blinks...");
    sos.start("... --- ...   ",-1);
  }
  
}
