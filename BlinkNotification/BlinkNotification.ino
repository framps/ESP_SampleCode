// -------------------------------------------------------------------------------------------------------------
// Sample how to use class BlinkNotification
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

using namespace framp;

#define LED_PIN 23

Ticker createError;                                 // ticker to simulate an error situation
bool errorOccured = false;
void setError() {
  errorOccured = true;
}

// active blink pattern to signal ESP is active working, no default
BlinkNotification activity(LED_PIN,                  // LED gpio to blink, no default
                            500,                     // period length of one LED blink in ms, default is 1000 ms
                            "-.",                    // blink pattern, default is "."
                            -1,                      // repeat loop 10 times, default is 0, use -1 for an endless loop
                            1000) ;                  // delay in ms at end of pattern, default is 0

// signal some error condition and blink 10 times
BlinkNotification error(LED_PIN,50,".",100,10);      // blink fast

// error blink pattern to signal an error condition, executed forever
BlinkNotification sos(LED_PIN,500,"... --- ...   ",-1);

void setup() {

  activity.start();                                 // start blink activity pattern to simulate some activity

  createError.once_ms(10000, setError);             // simulate error in 10 seconds

}

void loop() {

  if ( errorOccured ) {
    activity.stop();                                // terminate blink activity
    delay(1000);
    errorOccured = false;                           // don't execute this error path any more
    error.start();                                  // signal some error condition
    while (error.isActive()) {                      // wait until error condition is gone
      delay(100);
    }
    delay(1000);
    sos.start();                                    // blink the error pattern (SOS) three times
  }
}
