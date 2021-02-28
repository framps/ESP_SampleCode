// -------------------------------------------------------------------------------------------------------------
// Sample how to use class BlinkNotification
// -------------------------------------------------------------------------------------------------------------

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

using namespace framp;

Ticker createError;                                 // ticker to simulate an error situation
bool errorOccured = false;
void setError() {
  errorOccured = true;
}

BlinkNotification activity(BUILTIN_LED,250,".",-1);                    // active blink pattern to signal ESP is active working, executed forever
BlinkNotification sos(BUILTIN_LED,500,"... --- ...   ",3);             // error blink pattern to signal an error condition, execute three times 
 
void setup() {
  
  Serial.begin(115200);
  Serial.println();
  
  activity.start();                                 // start blink activity pattern

  createError.once_ms(10000, setError);             // simulate error in 10 seconds

}

void loop() {  

  if ( errorOccured ) {
    activity.stop();                                // terminate blink activity  
    errorOccured = false;                           // don't execute this error path any more
    delay(3000);                                    // separate blink patterns from each other
    
    sos.start();                                    // blink the error pattern (SOS) three times
  }
    
}
