//
// Sample sketch which uses a REED NO/NC switch to detect whether a new letter was inserted into the mailbox.
// If an ESP8266 is used additional electronic hardware is required. An ESP32 is able to handle multiple interupts
// which allows a simple state machine to handle everything without any complex additional HW (just two 100k pullup 
// resistors are needed for the two GPIOs used for NO and NC).
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

#define GPIO_FLAP_CLOSED 33
#define GPIO_FLAP_OPEN 15

#define BUTTON_PIN_BITMASK_FLAP_CLOSED 0x200000000 /* 2^33 - GPIO33 */
#define BUTTON_PIN_BITMASK_FLAP_OPENED 0x000008000 /* 2^15 - GPIO15 */

RTC_DATA_ATTR int state = 0;

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  5        /* Time ESP32 will go to sleep (in seconds) */

#define STATE_FLAP_CLOSED 0
#define STATE_FLAP_OPENED 1
#define STATE_FLAP_STILL_OPEN 2

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
  int64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  int GPIO;
  if ( GPIO_reason != 0 ) {
    GPIO = (log(GPIO_reason)) / log(2);
    Serial.print("GPIO that triggered the wake up: GPIO ");
    Serial.println(GPIO, 0);
  }
  return GPIO;
}

/*
  Return the state of the FLAP_OPEN GPIO
*/

int flapOpen() {
  return digitalRead(GPIO_FLAP_OPEN);
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
    case STATE_FLAP_CLOSED: Serial.print("<FLAP_CLOSED>"); break;
    case STATE_FLAP_OPENED: Serial.print("<FLAP_OPENED>"); break;
    case STATE_FLAP_STILL_OPEN: Serial.print("<FLAP_STILL_OPEN>"); break;
  }
}

/*
  Code for the different states
*/

void state_flapClosed() {

  if ( flapOpen() ) { // flap was open
      Serial.println("> Flap hast to be closed first ...");
      // enable timer to detect flap is closed and then enable opened interupt
      esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
      Serial.println("Setup ESP32 to sleep for " + String(TIME_TO_SLEEP) + " Seconds");
  } else {
    // enable flap open interupt
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("> Waiting for flap to open ...");  
    nextState(STATE_FLAP_OPENED);
  }
}

void state_flapOpened() {

  Serial.println("@@@ Mail received @@@");

  if ( ! flapOpen() ) { // flap was closed very fast
      Serial.println("> Flap detected to be closed already :-)");
      state_flapClosed();      
  } else {
    // enable flap close interupt
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED, ESP_EXT1_WAKEUP_ANY_HIGH);
    // enable timer to detect flap is still open because of long mail which causes the flap to stay open until mail is removed
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    Serial.println("Setup ESP32 to sleep for " + String(TIME_TO_SLEEP) + " Seconds");
    nextState(STATE_FLAP_STILL_OPEN);
  }
}

void state_flapStillOpen() {

  // flap was closed by interupt
  if ( print_GPIO_wake_up() == GPIO_FLAP_CLOSED ) {
    Serial.println("> Flap closed interupt received :-)");
    state_flapClosed();
  } else {  
    // flap still open, enable flap close interupt
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("> Waiting for flap to be closed ...");
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

  // pinMode(GPIO_FLAP_OPEN, INPUT_PULLUP);
  // pinMode(GPIO_FLAP_CLOSED, INPUT_PULLUP);

  print_GPIO_wake_up();
  print_wakeup_reason();

  Serial.print("Current state: "); printState(state); Serial.println();
  
  switch (state) {

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
