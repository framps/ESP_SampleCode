//
// Sample sketch which uses two REED NO/NC switches to detect whether a new letter was inserted into the mailbox.
// Given two GPIOs are uses in parallel to wakeup the ESP EXT1 is used and thus the code runs on an ESP32 only
//
// Requirement: 2 REED NO/NC contacts - one to detect flap open/close and one to detect door open to reset eMail notification
// Flap may stay opened if a long eMail causes the flap to stay open and no notification is sent until the door is opened 
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
#define GPIO_FLAP_OPENED 15
#define GPIO_DOOR_OPENED 32
#define GPIO_33_NUM GPIO_NUM_33
#define GPIO_15_NUM GPIO_NUM_15
#define GPIO_32_NUM GPIO_NUM_32

#define BUTTON_PIN_BITMASK_FLAP_CLOSED 0x200000000 /* 2^33 - GPIO33 */
#define BUTTON_PIN_BITMASK_DOOR_OPENED 0x100000000 /* 2^32 - GPIO32 */
#define BUTTON_PIN_BITMASK_FLAP_OPENED 0x000008000 /* 2^15 - GPIO15 */

RTC_DATA_ATTR int state = 0;
RTC_DATA_ATTR int notified = 0;

int newState;
int wakeupReason;
int gpio;

#define uS_TO_S_FACTOR 1000000              /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP_CHECK_OPEN  5         /* Time ESP32 will go to sleep (in seconds) to check if flap still open */

/*
  Return and optionally print the reason by which ESP32
  has been awaken from sleep
*/
esp_sleep_wakeup_cause_t wakeup_reason(int print) {

  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  if ( print ) {
    switch (wakeup_reason)
    {
      case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
      case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
      case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
      case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
      default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;
    }
  }
  return wakeup_reason;
}

/*
  Return and optionally print the GPIO which caused the interupt
*/

int GPIO_wake_up(int print) {
  int GPIO = -1;
  int64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
  if ( GPIO_reason == ESP_SLEEP_WAKEUP_EXT0 || GPIO_reason == ESP_SLEEP_WAKEUP_EXT1 ) {
    GPIO = (log(GPIO_reason)) / log(2);
  }
  if ( print ) {
    if ( GPIO != -1 ) {
      Serial.print("GPIO that triggered the wake up: GPIO ");
      Serial.println(GPIO, 0);    
    } else {
      Serial.println("Wake up not trigger by GPIO");
    }
  }
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

int doorOpen() {
  return digitalRead(GPIO_DOOR_OPENED);
}

void notifyEmail() {
  if ( ! notified ) {
    Serial.println("@@@ Mail received @@@");  
    notified=1;
  }
}

void resetNotifyEmail() {
  Serial.println("@@ Mail notify reset @@@");  
  notified=0;
}

// system booted

void initialState() {

  Serial.println("*** Initializing ***");

  state = flapOpen();

	if ( state ) { 
      Serial.println("> Open flap detected :-)");
      Serial.println("> Waiting for flap closed interupt ...");
      esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED | BUTTON_PIN_BITMASK_DOOR_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
	} else {
      Serial.println("> Waiting for flap opened interupt ...");
      esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED | BUTTON_PIN_BITMASK_DOOR_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
	}
}

void flapOpenToClose(int newState) {
  
  Serial.println("--- flapOpenToClose ---");

  int doorOpenedBit=0;

  if ( doorOpen() ) {
    resetNotifyEmail();
    doorOpenedBit = BUTTON_PIN_BITMASK_DOOR_OPENED;
  }

  if ( newState ) {
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED | doorOpenedBit, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("> Waiting for flap closed interupt ...");
  } else {
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED | doorOpenedBit, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("> Waiting for flap open interupt ...");
  }    
}

void flapCloseToOpen(int newState) {

  Serial.println("--- flapCloseToOpen ---");

  if ( doorOpen() ) {
    resetNotifyEmail();
  } else {
    notifyEmail();
    // enable timer to detect flap is still open because of long mail which causes the flap to stay open until mail is removed
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_CHECK_OPEN * uS_TO_S_FACTOR);
    Serial.println("Setup watchTimer to wake up in " + String(TIME_TO_SLEEP_CHECK_OPEN) + " seconds");
  }

  // enable flap close interupt
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED | BUTTON_PIN_BITMASK_DOOR_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
    
}

void flapCloseToClose(int newState) {

  Serial.println("--- flapCloseToClose ---");

  if ( doorOpen() ) {
    resetNotifyEmail();
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
  } else if (wakeupReason != ESP_SLEEP_WAKEUP_TIMER ) {
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED | BUTTON_PIN_BITMASK_DOOR_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
		notifyEmail();
	}
}

void flapOpenToOpen(int newState) {

    Serial.println("--- flapOpenToOpen ---");

    int doorOpenedBit=0;

    if ( doorOpen() ) {
      resetNotifyEmail();
    } else {
       doorOpenedBit = BUTTON_PIN_BITMASK_DOOR_OPENED;
    }

    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED | doorOpenedBit, ESP_EXT1_WAKEUP_ANY_HIGH);
    Serial.println("> Waiting for flap to be closed ...");    
  }

void flapWasInStateClosed(int newState) {

  switch (newState) {

	case 0:
	  flapCloseToClose(newState);
	  break;

	case 1:
	  flapCloseToOpen(newState);
	  break;
	}
}
	

void flapWasInStateOpened(int newState) {

  switch (newState) {

	case 0:
	  flapOpenToClose(newState);
	  break;

	case 1:
	  flapOpenToOpen(newState);
	  break;
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

  int newState = flapOpen();

  Serial.print("Current state: "); Serial.println(state);
  Serial.print("New state: "); Serial.println(newState);

  gpio = GPIO_wake_up(1);

  wakeupReason = wakeup_reason(1);
  if ( ! ( wakeupReason == ESP_SLEEP_WAKEUP_EXT1 || wakeupReason == ESP_SLEEP_WAKEUP_TIMER ) ) {
	
	  initialState();
	  
  } else {

	  switch (state) {

		case 0:
		  flapWasInStateClosed(newState);
		  break;

		case 1:
		  flapWasInStateOpened(newState);
		  break;

	  }
	}
	
  state = newState;

  //Go to sleep now
  Serial.print("Next state: "); Serial.println(newState);
  Serial.println("Going to sleep now");
  Serial.println("");

  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void loop() {
  //This is not going to be called
}
