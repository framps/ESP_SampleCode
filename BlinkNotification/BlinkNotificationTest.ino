#include "BlinkNotifications.h"

using namespace framp;

BlinkNotifications sos(BUILTIN_LED,250);

void setup() {
  Serial.begin(115200);
  sos.setup();

  Serial.println("SOS ...");
  sos.start("... --- ...   ",-1);
}

void loop() {
  sos.loop();
}
