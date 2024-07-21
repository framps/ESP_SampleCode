//
// Sample sketch which uses a REED NO/NC switch to detect a new letter was inserted into the mailbox
//
// Sketch can run on ESP8266 with EXT0 or ESP32 with EXT1
//
// Requirement: 1 REED NO/NC contact to detect flap open/close 
// Flap can stay open if a long eMail causes the flap to stay open and no notification is sent until the flap is close again
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

#include <PushoverESP32.h>

#define ESP8266 // EXT0 is used instead of EXT1

#define OPEN 1
#define CLOSED 0

#define GPIO_FLAP_CLOSED 33
#define GPIO_FLAP_OPENED 15
#define GPIO_FLAP_CLOSED_NUM GPIO_NUM_33
#define GPIO_FLAP_OPENED_NUM GPIO_NUM_15

#define BUTTON_PIN_BITMASK_FLAP_CLOSED 0x200000000 /* 2^33 - GPIO33 */
#define BUTTON_PIN_BITMASK_FLAP_OPENED 0x000008000 /* 2^15 - GPIO15 */

RTC_DATA_ATTR int state = 0;

int newState;   // state detected when woken up which may be different than the state when entering deep sleep
int wakeupReason; // time, boot or GPIO interupt
int gpio;       // gpio which caused the wakeup if an EXT0 or EXT1 was raised

#define uS_TO_S_FACTOR 1000000              /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP_CHECK_OPEN  5         /* Time ESP will go to sleep (in seconds) to check if flap still open */
    
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
   Return and optionally print the GPIO which caused the wakeup
 */

int GPIO_wake_up(int print) {
    int GPIO = -1;
#ifdef ESP8266
    if ( flapOpen() ) {
        GPIO = GPIO_FLAP_OPENED;
    } else {
        GPIO = GPIO_FLAP_CLOSED;
    }
#else  
    int64_t GPIO_reason = esp_sleep_get_ext1_wakeup_status();
    if ( GPIO_reason != 0 ) {
        GPIO = (log(GPIO_reason)) / log(2);
    }
#endif

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

void printState(int state) {
    if ( state ) {
        Serial.println("<OPEN>");
    } else {
        Serial.println("<CLOSED>");
    }    
}

void enableFlapClosedWakeup() {
    Serial.println("> Enabling flap closed wakeup ...");
#ifdef ESP8266      
    esp_sleep_enable_ext0_wakeup(GPIO_FLAP_CLOSED_NUM, 1);
#else
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif      
}

void enableFlapOpenedWakeup() {
    Serial.println("> Enabling flap opened wakeup ...");
#ifdef ESP8266      
    esp_sleep_enable_ext0_wakeup(GPIO_FLAP_OPENED_NUM, 1);
#else
    esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif      
}

void enableFlapStillOpenDetectTimer() {
    // enable timer to detect flap is still open because of long mail which causes the flap to stay open until mail is removed
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_CHECK_OPEN * uS_TO_S_FACTOR);
    Serial.println("Setup watchTimer to wake up in " + String(TIME_TO_SLEEP_CHECK_OPEN) + " seconds");  
}

int flapOpen() {
    return digitalRead(GPIO_FLAP_OPENED);
}

/*
 * Notify a new mail was received
 */

void notifyNewMail() {

    Serial.println("@@@ New mail received @@@");  

/*
    connectToWlan();

    char* token = "";
    char* user = "";

    Pushover pushoverClient(token, user);

    PushoverMessage myMessage;
    
    myMessage.title = "New Mail received";
    pushoverClient.send(myMessage);

    disconnectFromWlan();
*/    
      
}

// system booted

void initialState() {

    Serial.println("--- initialState ---");

    state = flapOpen();

    if ( state ) { 
        Serial.println("> Open flap detected :-)");
        enableFlapClosedWakeup();
    } else {
        Serial.println("> Closed flap detected :-)");
        enableFlapOpenedWakeup();
    }
}

void flapOpenToClose(int newState) {

    Serial.println("--- flapOpenToClose ---");

    if ( newState ) {
        enableFlapClosedWakeup();
    } else {
        enableFlapOpenedWakeup();
    }    
}

void flapCloseToOpen(int newState) {

    Serial.println("--- flapCloseToOpen ---");

    notifyNewMail();

    enableFlapStillOpenDetectTimer();
    enableFlapClosedWakeup();    
}

void flapCloseToClose(int newState) {

    Serial.println("--- flapCloseToClose ---");

    if (wakeupReason != ESP_SLEEP_WAKEUP_TIMER ) {
        enableFlapOpenedWakeup();
        notifyNewMail();
    }
}

void flapOpenToOpen(int newState) {

    Serial.println("--- flapOpenToOpen ---");

    enableFlapClosedWakeup();
}


/*
   Check which state is active now and call function which handles the new state
 */

void setup() {

    Serial.begin(115200);
    delay(1000); //Take some time to open up the Serial Monitor

    Serial.println("*****************************");
    Serial.println();

    int newState = flapOpen();

    Serial.print("Last state: "); printState(state);
    Serial.print("Current state: "); printState(newState);

    gpio = GPIO_wake_up(1);

    wakeupReason = wakeup_reason(1);
#ifndef ESP8266  
    if ( ! ( wakeupReason == ESP_SLEEP_WAKEUP_EXT1 || wakeupReason == ESP_SLEEP_WAKEUP_TIMER ) ) {
#else
        if ( ! ( wakeupReason == ESP_SLEEP_WAKEUP_EXT0 || wakeupReason == ESP_SLEEP_WAKEUP_TIMER ) ) {
#endif    

            initialState();

        } else {

            switch (state) {
                case CLOSED:
                    switch (newState) {
                        case CLOSED:
                            flapCloseToClose(newState);
                            break;

                        case OPEN:
                            flapCloseToOpen(newState);
                            break;
                    }
                    break;

                case OPEN:
                    switch (newState) {        
                        case CLOSED:
                            flapOpenToClose(newState);
                            break;

                        case OPEN:
                            flapOpenToOpen(newState);
                            break;
                    }
                    break;

            }
        }

        state = newState;

        //Go to sleep now
        Serial.print("Next state: "); printState(newState);
        Serial.println("Going to sleep now");
        Serial.println("");

        esp_deep_sleep_start();
        Serial.println("This will never be printed");
    }

    void loop() {
        //This is not going to be called
    }
