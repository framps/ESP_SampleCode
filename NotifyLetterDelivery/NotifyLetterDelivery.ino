//
// Sample sketch which uses a REED NO/NC switch to detect whether a new letter was inserted into the mailbox.
// Either EXT0 or EXT1 can be used so it can be used on a ESP8266 or ESP32.
//
// States:
//
// INITIAL: System was restarted and waits for flap top be closed if it's open
// FLAP_CLOSED: Enable open flap interupt and wait for letter -> change state to FLAP_OPENED
// FLAP_OPENED: flap was opened -> change state to FLAP_STILL_OPEN
// FLAP_STILL_OPEN: flap is still open, wait for flap closed interupt -> stay in state FLAP_STILL_OPEN, if flap was close start over -> change state to FLAP_CLOSED
//
// There may be interupts and state transitions not be detected because the ESP is not sleeping but not able to detect an interupt. These interupt holes are mitigated.
// 
// Following race conditions exist:
//
// FLAP_CLOSED: Flap quickly opened and closed but closed interupt not received
// FLAP_OPENED: Flap quickly closed and opened but opened interupt not received
// FLAP_STILL_OPEN: Flap quickly closed and opened but opened interupt not received
// All states first check for an external state change which may have been happened but was not detected and take care of the changed state
//
// Code based on the Arduino example code for ESP32_ExternalWakeup
//
// Latest code available on https://github.com/framps/ESP_stuff/
//

/*
#######################################################################################################################
#
#    Copyright (c) 2024 framp at linux-tips-and-tricks dot de
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

/*
#define EXT0 // use EXT0 instead of EXT1
*/

#define GPIO_FLAP_CLOSED 33
#define GPIO_FLAP_OPENED 15
#define GPIO_33 GPIO_NUM_33
#define GPIO_15 GPIO_NUM_15

#define BUTTON_PIN_BITMASK_FLAP_CLOSED 0x200000000 /* 2^33 - GPIO33 */
#define BUTTON_PIN_BITMASK_FLAP_OPENED 0x000008000 /* 2^15 - GPIO15 */

#define STATE_INITIAL 0
#define STATE_FLAP_CLOSED 1
#define STATE_FLAP_OPENED 2
#define STATE_FLAP_STILL_OPEN 3

RTC_DATA_ATTR int state = STATE_INITIAL;

#define uS_TO_S_FACTOR 1000000              /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP_CHECK_OPEN  5         /* Time ESP32 will go to sleep (in seconds) to check if flap still open */

/*
  Method to print the reason by which ESP32
  has been awaken from sleep
*/
esp_sleep_wakeup_cause_t print_wakeup_reason() {

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
  }
  return wakeup_reason;
}

/*
  Print and return the GPIO which caused the interupt
*/

int print_GPIO_wake_up() {
  int GPIO = -1;
#ifdef EXT0
  if ( flapOpen() ) {
      GPIO=GPIO_FLAP_OPENED;
  } else {
      GPIO=GPIO_FLAP_CLOSED;
  }
#else
  int64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  if ( GPIO_reason != 0 ) {
    GPIO = (log(GPIO_reason)) / log(2);
  }
#endif
  Serial.print("GPIO that triggered the wake up: GPIO ");
  Serial.println(GPIO, 0);
  return GPIO;
}

/*
  Return the state of the FLAP_OPEN GPIO
*/

int flapOpen() {
  return digitalRead(GPIO_FLAP_OPENED);
}

/*
  Return the state of the FLAP_CLOSED GPIO
*/

int flapClosed() {
  return digitalRead(GPIO_FLAP_CLOSED);
}

/*
  Set state which is used when next interupt occurs
*/

void nextState(int nextState) {
  state = nextState;
}

/*
  Print the current state
*/

void printState(int state) {
  switch (state) {
    case STATE_INITIAL: Serial.print("<INITIAL>"); break;
    case STATE_FLAP_CLOSED: Serial.print("<FLAP_CLOSED>"); break;
    case STATE_FLAP_OPENED: Serial.print("<FLAP_OPENED>"); break;
    case STATE_FLAP_STILL_OPEN: Serial.print("<FLAP_STILL_OPEN>"); break;
  }
}

/*
  Code for the different states
*/

// system booted

void state_initial() {

  // no interupt happend
  // just check whether flap is open

  if ( flapOpen() ) { 
      Serial.println("> Open flap detected :-)");
	  // now enable flap closeinterupt
  #ifdef EXT0
    esp_sleep_enable_ext0_wakeup(GPIO_33,1); //1 = High, 0 = Low
  #else
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED, ESP_EXT1_WAKEUP_ANY_HIGH);
  #endif
    Serial.println("> Waiting now for flap close interupt ...");
    nextState(STATE_FLAP_CLOSED);
  } else {
	// assume now flap is clsed
    state_flapClosed();
  }
}

// flap was closed

void state_flapClosed() {

  // detected by closed interupt or set by initial state
  // assumes flap was open previously

  if ( flapOpen() ) { // flap was opened very fast
      Serial.println("> Open flap detected :-)");
      state_flapOpened(); // change state to opened
  } else {
    // enable flap open interupt
  #ifdef EXT0
    esp_sleep_enable_ext0_wakeup(GPIO_15,1); //1 = High, 0 = Low
  #else
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
  #endif
    Serial.println("> Waiting for flap open interupt ...");
    nextState(STATE_FLAP_OPENED);
  }
}

// Flap was opened, deceted by opened interupt, wait for closed interupt or timer

void state_flapOpened() {

  Serial.println("@@@ Mail received @@@");

  if ( flapClosed() ) { // flap was closed 
      Serial.println("> Flap detected to be closed already :-)");
      state_flapClosed(); // change state to closed
  } else {
    // enable flap close interupt
#ifdef EXT0
    esp_sleep_enable_ext0_wakeup(GPIO_33,1); //1 = High, 0 = Low
#else
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
    // enable timer to detect flap is still open because of long mail which causes the flap to stay open until mail is removed
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_CHECK_OPEN * uS_TO_S_FACTOR);
    Serial.println("Setup watchTimer to wake up in " + String(TIME_TO_SLEEP_CHECK_OPEN) + " seconds");
    nextState(STATE_FLAP_STILL_OPEN);
  }
}

// flap is still open, detected by timer interupt or close interupt

void state_flapStillOpen() {

  if ( flapClosed() ) {
    Serial.println("> Flap detected to be closed already :-)");
    state_flapClosed();
  // flap was closed by interupt or is detected by timer
  } else {
    // flap still open, enable flap close interupt
#ifdef EXT0
    esp_sleep_enable_ext0_wakeup(GPIO_33,1); //1 = High, 0 = Low
#else
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
    Serial.println("> Waiting for flap to be closed ...");
    
    nextState(STATE_FLAP_STILL_OPEN);
  }
}

/*
  Check which state is active and call according function
*/

void setup() {
  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  Serial.println("*****************************");
  Serial.println();

  // pinMode(GPIO_FLAP_OPEN, INPUT_PULLDOWN);
  // pinMode(GPIO_FLAP_CLOSED, INPUT_PULLDOWN);

  int wakeupReason = print_wakeup_reason();
  if ( wakeupReason == ESP_SLEEP_WAKEUP_EXT0 || wakeupReason == ESP_SLEEP_WAKEUP_EXT1 ) {
    print_GPIO_wake_up();
  }

  Serial.print("Current state: "); printState(state); Serial.println();

  switch (state) {

    case STATE_INITIAL:
      state_initial();
      break;

    case STATE_FLAP_CLOSED:
      state_flapClosed();
      break;

    case STATE_FLAP_OPENED:
      state_flapOpened();
      break;

    case STATE_FLAP_STILL_OPEN:
      state_flapStillOpen();
      break;

  }

  //Go to sleep now
  Serial.print("Next state: "); printState(state); Serial.println();
  Serial.println("Going to sleep now");
  Serial.println("");

  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  //This is not going to be called
}
