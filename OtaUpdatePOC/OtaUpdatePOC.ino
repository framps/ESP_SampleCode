// -------------------------------------------------------------------------------------------------------------
// Proof of concept how to update an ESP image with OTA
//
// Algorithm:
// Compare the image file LastUpdated header field with the build date of the image.
// If it's newer than the build date update the ESP
// -------------------------------------------------------------------------------------------------------------

/*
#######################################################################################################################
#
#    Copyright (c) 2022 framp at linux-tips-and-tricks dot de
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

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

#include <Timezone.h>    // https://github.com/JChristensen/Timezone

#include "environment.h" // defines AP_NAME and AP_PASSWORD for WLAN login

const char *buildTime = __TIME__;
const char *buildDate = __DATE__;

const char* fwUrlBase = "http://192.168.0.12/iot/";       // image download location, should be named <FILENAME>.bin (.ino will be replaced with .bin)

const char * headerkeys[] = {"Last-Modified"};
const size_t headerkeyssize = sizeof(headerkeys)/sizeof(char*);

// Timezone my Arduino runs in and creates the build date and time
// Central European Time (Frankfurt, Paris)
TimeChangeRule CEST = {"CEST", Last, Sun, Mar, 2, 120};     // Central European Summer Time
TimeChangeRule CET = {"CET ", Last, Sun, Oct, 3, 60};       // Central European Standard Time
Timezone CE(CEST, CET);

ESP8266WiFiMulti WiFiMulti;

// Following function borrowed and modified from https://github.com/JChristensen/Timezone/tree/master/examples/WorldClock
// Function to return the compile date and time as a time_t value
time_t compileTime(const char* buildTime, const char* buildDate) {
    const time_t FUDGE(10);     // fudge factor to allow for compile time (seconds, YMMV)
    const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
    char chMon[4], *m;
    tmElements_t tm;

    strncpy(chMon, buildDate, 3);
    chMon[3] = '\0';
    m = strstr(months, chMon);
    tm.Month = ((m - months) / 3 + 1);

    tm.Day = atoi(buildDate + 4);
    tm.Year = atoi(buildDate + 7) - 1970;
    tm.Hour = atoi(buildTime);
    tm.Minute = atoi(buildTime + 3);
    tm.Second = atoi(buildTime + 6);
    time_t t = makeTime(tm);
    return t + FUDGE;           // add fudge factor to allow for compile time
}

void checkForUpdates() {

  String fwURL = String( fwUrlBase );
  fwURL.concat( __FILE__ );
  fwURL.replace(".ino",".bin");             // replace FILENAME .ino to .bin for image to check

  Serial.printf( "Firmware build: %s %s\n", buildDate, buildTime );
  Serial.printf( "Firmware update download URL: %s\n", fwURL.c_str() );

  WiFiClient wifiClient;
  HTTPClient httpClient;

  httpClient.begin( wifiClient, fwURL);

  // check if image is newer than current image build with HEAD request
  httpClient.collectHeaders(headerkeys, headerkeyssize);

  int httpCode=httpClient.sendRequest("HEAD","");

  if( httpCode == 200 ) {

    // Last-Modified = Sat, 30 Jul 2022 21:12:09 GMT
     String lastModified=httpClient.header("Last-Modified");
     Serial.printf("Last-Modified of firmware image: %s\n",lastModified.c_str());

    // convert Last-Modified format which is according RFC in UTC to format accepted by compileTime
    // so that compileTime and LastUpdatedTime can be compared

     char lm[256];
     char buffer[256];

     strcpy(lm,lastModified.c_str());
     lm[25]='\0';
     sprintf(buffer, "%s",
       lm+17       // hh:mm:ss
       );
     String imageTime(buffer);

     strcpy(lm,lastModified.c_str());
     lm[11]='\0';
     sprintf(buffer, "%s  %i %i",
       lm+8,       // month
       atoi(lm+5), // day
       atoi(lm+12) // year
       );
     String imageDate(buffer);

     time_t compileCE = compileTime(buildTime, buildDate);
     time_t compileUTC=CE.toUTC(compileCE);               // convert compile time to UTC in local CE timezone
     time_t imageUTC = compileTime(imageTime.c_str(), imageDate.c_str());

    double timeDifference = difftime(imageUTC, compileUTC);
    Serial.printf("timeDifference: %f\n",timeDifference);

    if( timeDifference > 60.0 ) { // equal comparison of double for zero doesn't make sense, this time difference in seconds is considered as an update

      Serial.println( "New firmware found. Preparing to update firmware" );

      t_httpUpdate_return ret = ESPhttpUpdate.update( wifiClient, fwURL );

      switch(ret) {
        case HTTP_UPDATE_FAILED:
          Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
          break;
        case HTTP_UPDATE_NO_UPDATES:
          Serial.println("HTTP_UPDATE_NO_UPDATES");
          break;
        case HTTP_UPDATE_OK:
          Serial.println("HTTP_UPDATE_OK");
          break;
        default:
          Serial.printf("Unknown HTTP_UPDATE rc received: ");
          Serial.println(ret);
          break;
      }
    }
    else {
      Serial.println( "Old or current firmware detected" );
    }
  }
  else {
    Serial.print( "Firmware version check failed, got HTTP response code " );
    Serial.println( httpCode );
  }

  httpClient.end();

}

void setup() {

  // put your setup code here, to run once:

  Serial.begin(115200);
  delay(1000);
  Serial.println();

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(AP_NAME, AP_PASSWORD);

  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.println("Connecting to WLAN ...");
    delay(3000);
  };

  checkForUpdates();
}

void loop() {

  // put your main code here, to run repeatedly:

  for (int i=0; i<3; i++) {
    Serial.println("Working...");
    delay(5000);
  }
  ESP.restart();
}
