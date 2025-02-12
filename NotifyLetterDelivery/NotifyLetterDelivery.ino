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
  #    Copyright (c) 2024-2025 framp at linux-tips-and-tricks dot de
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

// #define DEBUG
# define USE_PUSHOVER
# define USE_EMAIL

#ifdef USE_PUSHOVER
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

//Pushover API endpoint
const char* pushoverApiEndpoint = "https://api.pushover.net/1/messages.json";

//Pushover root certificate (valid from 11/10/2006 to 15/01/2038)
const char *PUSHOVER_ROOT_CA = "-----BEGIN CERTIFICATE-----\n"
                               "MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh\n"
                               "MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\n"
                               "d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH\n"
                               "MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT\n"
                               "MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j\n"
                               "b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG\n"
                               "9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI\n"
                               "2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx\n"
                               "1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ\n"
                               "q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz\n"
                               "tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ\n"
                               "vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP\n"
                               "BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV\n"
                               "5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY\n"
                               "1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4\n"
                               "NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG\n"
                               "Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91\n"
                               "8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe\n"
                               "pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl\n"
                               "MrY=\n"
                               "-----END CERTIFICATE-----\n";
#endif

#ifdef USE_EMAIL
#include <ESP_Mail_Client.h>

SMTPSession smtp;
void smtpCallback(SMTP_Status status);
#endif

#include <homedefs.h>
  
#ifdef ESP_SLEEP_WAKEUP_EXT0
#define ESP8266 // EXT0 is used instead of EXT1
#endif

#define OPEN 1
#define CLOSED 0

#define GPIO_FLAP_CLOSED 33
#define GPIO_FLAP_OPENED 15
#define GPIO_FLAP_CLOSED_NUM GPIO_NUM_33
#define GPIO_FLAP_OPENED_NUM GPIO_NUM_15

#define BUTTON_PIN_BITMASK_FLAP_CLOSED 0x200000000 /* 2^33 - GPIO33 */
#define BUTTON_PIN_BITMASK_FLAP_OPENED 0x000008000 /* 2^15 - GPIO15 */

RTC_DATA_ATTR int state = 0;

int newState;               // state detected when woken up which may be different than the state when entering deep sleep
int wakeupReason;           // timeout, boot or GPIO interupt
int gpioCausedWakeup;       // gpio which caused the wakeup if an EXT0 or EXT1 was raised

int sleepWakeup =
#ifdef ESP_8266
  ESP_SLEEP_WAKEUP_EXT0;
#else
  ESP_SLEEP_WAKEUP_EXT1;
#endif

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
      case ESP_SLEEP_WAKEUP_EXT0 : 
        #ifdef DEBUG
        Serial.println("Wakeup caused by external signal using RTC_IO"); 
        #endif
        break;
      case ESP_SLEEP_WAKEUP_EXT1 : 
        #ifdef DEBUG
        Serial.println("Wakeup caused by external signal using RTC_CNTL"); 
        #endif
        break;
      case ESP_SLEEP_WAKEUP_TIMER : 
        #ifdef DEBUG
        Serial.println("Wakeup caused by timer"); 
        #endif
        break;
      case ESP_SLEEP_WAKEUP_TOUCHPAD : 
        #ifdef DEBUG
        Serial.println("Wakeup caused by touchpad"); 
        #endif
        break;
      case ESP_SLEEP_WAKEUP_ULP : 
        #ifdef DEBUG
        Serial.println("Wakeup caused by ULP program");
        #endif
        break;
      default : 
        #ifdef DEBUG
        Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); 
        #endif
        break;
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
      #ifdef DEBUG
      Serial.print("GPIO that triggered the wake up: GPIO ");
      Serial.println(GPIO, 0);
      #endif
    } else {
      #ifdef DEBUG
      Serial.println("Wake up not trigger by GPIO");
      #endif
    }
  }
  return GPIO;
}

void printState(int state) {
  if ( state ) {
    #ifdef DEBUG
    Serial.println("<OPEN>");
    #endif
  } else {
    #ifdef DEBUG
    Serial.println("<CLOSED>");
    #endif
  }
}

void enableFlapClosedWakeup() {
  #ifdef DEBUG
  Serial.println("> Enabling flap closed wakeup ...");
  #endif
#ifdef ESP8266
  esp_sleep_enable_ext0_wakeup(GPIO_FLAP_CLOSED_NUM, 1);
#else
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_CLOSED, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
}

void enableFlapOpenedWakeup() {
  #ifdef DEBUG
  Serial.println("> Enabling flap opened wakeup ...");
  #endif
#ifdef ESP8266
  esp_sleep_enable_ext0_wakeup(GPIO_FLAP_OPENED_NUM, 1);
#else
  esp_sleep_enable_ext1_wakeup(BUTTON_PIN_BITMASK_FLAP_OPENED, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
}

void enableFlapStillOpenDetectTimer() {
  // enable timer to detect flap is still open because of long mail which causes the flap to stay open until mail is removed
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP_CHECK_OPEN * uS_TO_S_FACTOR);
  #ifdef DEBUG
  Serial.println("Setup watchTimer to wake up in " + String(TIME_TO_SLEEP_CHECK_OPEN) + " seconds");
  #endif
}

int flapOpen() {
  return digitalRead(GPIO_FLAP_OPENED);
}

/*
   Notify a new mail was received
*/

void notifyNewMail() {

  Serial.println("@@@ New mail received @@@");

#ifdef USE_PUSHOVER
  connectToWLAN();
  notifyPushover("Mail received","Check your mailbox");
#ifndef USE_EMAIL
  WiFi.disconnect();
#endif
#endif

#ifdef USE_EMAIL
  #ifndef USE_PUSHOVER
  connectToWLAN();
  #endif
  sendEmail("Einwurf am Briefkasten entdeckt","Bitte Briefkasten leeren");
  #ifndef USE_PUSHOVER
  WiFi.disconnect();
  #endif
#endif
}

void connectToWLAN() {

  String ssid=AP_NAME;
  String pass=AP_PASSWORD;
  
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WLAN ");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WLAN");
}

// system booted

void initialState() {

  #ifdef DEBUG
  Serial.println("--- initialState ---");
  #endif

  state = flapOpen();

  if ( state ) {
    #ifdef DEBUG
    Serial.println("> Open flap detected :-)");
    #endif
    enableFlapClosedWakeup();
  } else {
    #ifdef DEBUG
    Serial.println("> Closed flap detected :-)");
    #endif
    enableFlapOpenedWakeup();
  }
}

void flapOpenToClose(int newState) {

  #ifdef DEBUG
  Serial.println("--- flapOpenToClose ---");
  #endif

  if ( newState ) {
    enableFlapClosedWakeup();
  } else {
    enableFlapOpenedWakeup();
  }
}

void flapCloseToOpen(int newState) {

  #ifdef DEBUG
  Serial.println("--- flapCloseToOpen ---");
  #endif

  notifyNewMail();

  enableFlapStillOpenDetectTimer();
  enableFlapClosedWakeup();
}

void flapCloseToClose(int newState) {

  #ifdef DEBUG
  Serial.println("--- flapCloseToClose ---");
  #endif

  if (wakeupReason != ESP_SLEEP_WAKEUP_TIMER ) {
    enableFlapOpenedWakeup();
    notifyNewMail();
  }
}

void flapOpenToOpen(int newState) {

  #ifdef DEBUG
  Serial.println("--- flapOpenToOpen ---");
  #endif

  enableFlapClosedWakeup();
}


/*
   Check which state is active now and call function which handles the new state
*/

void setup() {

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  Serial.begin(115200);
  delay(1000); //Take some time to open up the Serial Monitor

  #ifdef DEBUG
  Serial.println("******SETUP********");
  Serial.println();
  #endif

  // pinMode(GPIO_FLAP_CLOSED_NUM, INPUT_PULLDOWN);
  // pinMode(GPIO_FLAP_OPENED_NUM, INPUT_PULLDOWN);

  int newState = flapOpen();

  #ifdef DEBUG
  Serial.print("Last state: "); printState(state);
  Serial.print("Current state: "); printState(newState);
  #endif

  gpioCausedWakeup = GPIO_wake_up(1);

  wakeupReason = wakeup_reason(1);
  if ( ! ( wakeupReason == sleepWakeup || wakeupReason == ESP_SLEEP_WAKEUP_TIMER ) ) {
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
  #ifdef DEBUG
  Serial.print("Next state: "); printState(newState);
  #endif
  Serial.println("Going to sleep now");
  #ifdef DEBUG
  Serial.println("");
  #endif
  esp_deep_sleep_start();
}

void loop() {
  //This is not going to be called
}

#ifdef USE_PUSHOVER
void notifyPushover(char* title, char* message) {
	
  Serial.println("Notifying pushover ...");

  StaticJsonDocument<512> notification;
  notification["token"] = PUSHOVER_APITOKEN; //required
  notification["user"] = PUSHOVER_USERTOKEN; //required
  notification["message"] = message; //required
  notification["title"] = title; //optional
  /*
    notification["url"] = ""; //optional
    notification["url_title"] = ""; //optional
    notification["html"] = ""; //optional
    notification["priority"] = ""; //optional
    notification["sound"] = "cosmic"; //optional
    notification["timestamp"] = ""; //optional
  */

  // Serialize the JSON object to a string
  String jsonStringNotification;
  serializeJson(notification, jsonStringNotification);

  // Create a WiFiClientSecure object
  WiFiClientSecure client;
  // Set the certificate
  client.setCACert(PUSHOVER_ROOT_CA);

  // Create an HTTPClient object
  HTTPClient https;

  // Specify the target URL
  https.begin(client, pushoverApiEndpoint);

  // Add headers
  https.addHeader("Content-Type", "application/json");

  // Send the POST request with the JSON data
  int httpResponseCode = https.POST(jsonStringNotification);

  // Check the response
  if (httpResponseCode > 0) {
    #ifdef DEBUG
    Serial.printf("HTTP response code: %d\n", httpResponseCode);
    String response = https.getString();
    Serial.println("Response:");
    Serial.println(response);
    #endif
  } else {
    Serial.printf("Pushover failed with HTTP response code: %d\n", httpResponseCode);
  }

  // Close the connection
  https.end();
  Serial.println("Notified pushover");
}
#endif

#ifdef USE_EMAIL

void sendEmail(char* subject, char* content) {
   /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(0);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = EMAIL_SERVER;
  config.server.port = EMAIL_SERVER_PORT;
  config.login.email = EMAIL_USER;
  config.login.password = EMAIL_USER_LOGIN_PWD;

  /** Assign your host name or you public IPv4 or IPv6 only
   * as this is the part of EHLO/HELO command to identify the client system
   * to prevent connection rejection.
   * If host name or public IP is not available, ignore this or
   * use loopback address "127.0.0.1".
   *
   * Assign any text to this option may cause the connection rejection.
   */
  config.login.user_domain = F("127.0.0.1");

  /** If non-secure port is prefered (not allow SSL and TLS connection), use
   *  config.secure.mode = esp_mail_secure_mode_nonsecure;
   *
   *  If SSL and TLS are always required, use */
   config.secure.mode = esp_mail_secure_mode_ssl_tls;
   /*
   *  To disable SSL permanently (use less program space), define ESP_MAIL_DISABLE_SSL in ESP_Mail_FS.h
   *  or Custom_ESP_Mail_FS.h
   */
  // config.secure.mode = esp_mail_secure_mode_nonsecure;

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;

  /* The full message sending logs can now save to file */
  /* Since v3.0.4, the sent logs stored in smtp.sendingResult will store only the latest message logs */
  // config.sentLogs.filename = "/path/to/log/file";
  // config.sentLogs.storage_type = esp_mail_file_storage_type_flash;

  /** In ESP32, timezone environment will not keep after wake up boot from sleep.
   * The local time will equal to GMT time.
   *
   * To sync or set time with NTP server with the valid local time after wake up boot,
   * set both gmt and day light offsets to 0 and assign the timezone environment string e.g.
   */
     config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
     config.time.gmt_offset = 0;
     config.time.day_light_offset = 0;
     config.time.timezone_env_string = "CET-1"; // for Berlin
/*
   * The library will get (sync) the time from NTP server without GMT time offset adjustment
   * and set the timezone environment variable later.
   *
   * This timezone environment string will be stored to flash or SD file named "/tze.txt"
   * which set via config.time.timezone_file.
   *
   * See the timezone environment string list from
   * https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv
   *
   */

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = F("Dein Briefkasten zu Hause");
  message.sender.email = EMAIL_SENDER_MAIL;

  /** If author and sender are not identical
  message.sender.name = F("Sender");
  message.sender.email = "sender@mail.com";
  message.author.name = F("ESP Mail");
  message.author.email = AUTHOR_EMAIL; // should be the same email as config.login.email
 */

  // In case of sending non-ASCII characters in message envelope,
  // that non-ASCII words should be encoded with proper charsets and encodings
  // in form of `encoded-words` per RFC2047
  // https://datatracker.ietf.org/doc/html/rfc2047

  message.subject = subject;

  message.addRecipient("Briefkasten",EMAIL_SENDER_MAIL);

  message.text.content = content;

  /** The content transfer encoding e.g.
   * enc_7bit or "7bit" (not encoded)
   * enc_qp or "quoted-printable" (encoded)
   * enc_base64 or "base64" (encoded)
   * enc_binary or "binary" (not encoded)
   * enc_8bit or "8bit" (not encoded)
   * The default value is "7bit"
   */

  message.text.transfer_encoding = "base64"; // recommend for non-ASCII words in message.

  /** If the message to send is a large string, to reduce the memory used from internal copying  while sending,
   * you can assign string to message.text.blob by cast your string to uint8_t array like this
   *
   * String myBigString = "..... ......";
   * message.text.blob.data = (uint8_t *)myBigString.c_str();
   * message.text.blob.size = myBigString.length();
   *
   * or assign string to message.text.nonCopyContent, like this
   *
   * message.text.nonCopyContent = myBigString.c_str();
   *
   * Only base64 encoding is supported for content transfer encoding in this case.
   */

  /** The Plain text message character set e.g.
   * us-ascii
   * utf-8
   * utf-7
   * The default value is utf-8
   */
  message.text.charSet = F("utf-8"); // recommend for non-ASCII words in message.

  // If this is a reply message
  // message.in_reply_to = "<parent message id>";
  // message.references = "<parent references> <parent message id>";

  /** The message priority
   * esp_mail_smtp_priority_high or 1
   * esp_mail_smtp_priority_normal or 3
   * esp_mail_smtp_priority_low or 5
   * The default value is esp_mail_smtp_priority_low
   */
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  // message.response.reply_to = "someone@somemail.com";
  // message.response.return_path = "someone@somemail.com";

  /** The Delivery Status Notifications e.g.
   * esp_mail_smtp_notify_never
   * esp_mail_smtp_notify_success
   * esp_mail_smtp_notify_failure
   * esp_mail_smtp_notify_delay
   * The default value is esp_mail_smtp_notify_never
   */
  // message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  // // message.addHeader(F("Message-ID: <abcde.fghij@gmail.com>"));

  // For Root CA certificate verification (ESP8266 and ESP32 only)
  // config.certificate.cert_data = rootCACert;
  // or
  // config.certificate.cert_file = "/path/to/der/file";
  // config.certificate.cert_file_storage_type = esp_mail_file_storage_type_flash; // esp_mail_file_storage_type_sd
  // config.certificate.verify = true;

  // The WiFiNINA firmware the Root CA certification can be added via the option in Firmware update tool in Arduino IDE

  /* Connect to server with the session config */

  // Library will be trying to sync the time with NTP server if time is never sync or set.
  // This is 10 seconds blocking process.
  // If time reading was timed out, the error "NTP server time reading timed out" will show via debug and callback function.
  // You can manually sync time by yourself with NTP library or calling configTime in ESP32 and ESP8266.
  // Time can be set manually with provided timestamp to function smtp.setSystemTime.

  /* Set the TCP response read timeout in seconds */
  // smtp.setTCPTimeout(10);

  /* Connect to the server */
  if (!smtp.connect(&config))
  {
    MailClient.printf("Connection error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  /** Or connect without log in and log in later

     if (!smtp.connect(&config, false))
       return;

     if (!smtp.loginWithPassword(AUTHOR_EMAIL, AUTHOR_PASSWORD))
       return;
  */

  if (!smtp.isLoggedIn())
  {
    Serial.println("Not yet logged in.");
  }
  else
  {
    if (smtp.isAuthenticated())
      #ifdef DEBUG
      Serial.println("Successfully logged in.");
      #endif
      ; // noop
    else
      #ifdef DEBUG
      Serial.println("Connected with no Auth.");
      #endif
      ; // noop
  }

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    MailClient.printf("Error, Status Code: %d, Error Code: %d, Reason: %s\n", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());

  // to clear sending result log
  // smtp.sendingResult.clear();
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    // MailClient.printf used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    MailClient.printf("Message sent success: %d\n", status.completedCount());
    MailClient.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

      MailClient.printf("Message No: %d\n", i + 1);
      MailClient.printf("Status: %s\n", result.completed ? "success" : "failed");
      MailClient.printf("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
      MailClient.printf("Recipient: %s\n", result.recipients.c_str());
      MailClient.printf("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}
#endif
