//
// ESP32 RFID-Reader with PN532 relaisboard, webserver and email notification
//
// Code donated by fred0815 in following thread in German raspberryforum
// https://forum-raspberrypi.de/forum/thread/56590-esp32-rfid-leser-mit-pn532-relaisboard-webserver-und-emailbenachrichtigung/
//
// Code is provided as is with no support. Just use the code as a template.
//

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <PN532.h>
#include <PN532_debug.h>
#include <PN532Interface.h>
#include <string.h>
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <ESP_Mail_Client.h>
#include <FS.h>
#include <SPIFFS.h>

#define SMTP_HOST "mail.gmx.net"
#define SMTP_PORT 587
#define AUTHOR_EMAIL "absender@gmx.de"
#define AUTHOR_PASSWORD "geheim"
#define RECIPIENT_EMAIL "empfaenger@gmx.de"

SMTPSession smtp;
void smtpCallback(SMTP_Status status);

SoftwareSerial SWSerial( 22, 23 ); // RX, TX

PN532_SWHSU pn532swhsu( SWSerial );

PN532 nfc( pn532swhsu );

const char* ssid = "WLAN_SSID";
const char* password = "geheim";

IPAddress local_IP(192, 168, 0, 99);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);
IPAddress secondaryDNS(8, 8, 4, 4);

const char* input_parameter = "state";

const int output = 16;
const int Push_button_GPIO = 39 ;

// Variables will change:
int LED_state = HIGH;         
int button_state;             
int lastbutton_state = LOW;   

unsigned long lastDebounceTime = 0;  
unsigned long debounceDelay = 50;    

AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML>
<html lang="de">
<head>
  <title>Türöffner</title>
  <meta charset="utf-8" name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {font-family: Times New Roman; display: inline-block; text-align: center;}
    h2 {font-size: 3.0rem;}
    h4 {font-size: 2.0rem;}
    p {font-size: 3.0rem;}
    body {max-width: 900px; margin:0px auto; padding-bottom: 25px; background-color: #000000; color: #ffffff;}
    .switch {position: relative; display: inline-block; width: 120px; height: 68px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #FF0000; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 52px; width: 52px; left: 8px; bottom: 8px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #27c437}
    input:checked+.slider:before {-webkit-transform: translateX(52px); -ms-transform: translateX(52px); transform: translateX(52px)}
  </style>
</head>
<body>
  <h2>Türöffner</h2>
  %BUTTONPLACEHOLDER%
<script>function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/update?state=1", true); }
  else { xhr.open("GET", "/update?state=0", true); }
  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var inputChecked;
      var outputStateM;
      if( this.responseText == 1){ 
        inputChecked = true;
        outputStateM = "ZU";
      }
      else { 
        inputChecked = false;
        outputStateM = "AUF";
      }
      document.getElementById("output").checked = inputChecked;
      document.getElementById("outputState").innerHTML = outputStateM;
    }
  };
  xhttp.open("GET", "/state", true);
  xhttp.send();
}, 1000 ) ;
</script>
</body>
</html>
)rawliteral";

String processor(const String& var){
  if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    String outputStateValue = outputState();
    buttons+= "<h4>TÜRE IST <span id=\"outputState\"></span></h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"output\" " + outputStateValue + "><span class=\"slider\"></span></label>";
    return buttons;
  }
  return String();
}

String outputState(){
  if(digitalRead(output)){
    return "checked";
  }
  else {
    return "";
  }
  return "";
}


/* Callback function to get the Email sending status */

void smtpCallback(SMTP_Status status){

/* Print the current status */

Serial.println(status.info());

/* Print the sending result */ if (status.success()){

Serial.println("----------------");

ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());

ESP_MAIL_PRINTF("Message sent failled: %d\n", status.failedCount());

Serial.println("----------------\n"); struct tm dt;

for (size_t i = 0; i < smtp.sendingResult.size(); i++){

/* Get the result item */

SMTP_Result result = smtp.sendingResult.getItem(i);

time_t ts = (time_t)result.timestamp; localtime_r(&ts, &dt);

ESP_MAIL_PRINTF("Message No: %d\n", i + 1);

ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed"); ESP_MAIL_PRINTF("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900,

dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec); ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients); ESP_MAIL_PRINTF("Subject: %s\n", result.subject);

}}}

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
Serial.println("Starte Programm ...");

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();

  if (! versiondata) {

    Serial.print("Kein RFID-Modul gefunden !");

  }

  Serial.print("RFID gefunden:"); Serial.println((versiondata>>24) & 0xFF, HEX);

  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);

  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  nfc.SAMConfig();

  Serial.println("Warte auf ISO14443A Karte ...");
  
  pinMode(output, OUTPUT);
  digitalWrite(output, HIGH);
  pinMode(Push_button_GPIO, INPUT);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS);
  if (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Verbinde WLAN..");
  }


  Serial.println("IP Adresse: ");
  Serial.println(WiFi.localIP());


  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  
  server.on("/update", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String input_message;
    String inputParameter;
    // GET input1 value on <ESP_IP>/update?state=<input_message>
    if (request->hasParam(input_parameter)) {
      input_message = request->getParam(input_parameter)->value();
      inputParameter = input_parameter;
      digitalWrite(output, input_message.toInt());
      LED_state = !LED_state;
    }
    else {
      input_message = "No message sent";
      inputParameter = "none";
    }
    Serial.println(input_message);
    request->send(200, "text/plain", "OK");
  });

  
  server.on("/state", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send(200, "text/plain", String(digitalRead(output)).c_str());
  });
  // Start server
  server.begin();
}
  
void loop() {

  boolean success;

  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID

  uint8_t uidLength;                       // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &uid[0], &uidLength);

  if (success) {

    Serial.print("UID Länge: ");Serial.print(uidLength, DEC);Serial.println(" bytes");

    Serial.print("UID: ");

    for (uint8_t i=0; i < uidLength; i++)

    {

      Serial.print(uid[i], HEX);
      Serial.println("");
    }
  
    Serial.println("");

  uint8_t code1[4] = { 0x00, 0x00, 0x00, 0x01 } ;
  uidLength = 4 ;

  uint8_t code2[4] = { 0x00, 0x00, 0x00, 0x02 } ;
  uidLength = 4 ;

  uint8_t code3[4] = { 0x00, 0x00, 0x00, 0x03 } ;
  uidLength = 4 ;  

  uint8_t code4[4] = { 0x00, 0x00, 0x00, 0x04 } ;
  uidLength = 4 ;
  
  uint8_t code5[4] = { 0x00, 0x00, 0x00, 0x05 } ;
  uidLength = 4 ;

  uint8_t code6[4] = { 0x00, 0x00, 0x00, 0x06 } ;
  uidLength = 4 ;
    
if( memcmp(uid,code1,uidLength) == 0 )
{
   Serial.println( "Chip 1" ) ;
   pinMode(16, OUTPUT);   
   digitalWrite(16, LOW);
   pinMode(17, OUTPUT);   
   digitalWrite(17, LOW);
   pinMode(18, OUTPUT);   
   digitalWrite(18, LOW);
   pinMode(19, OUTPUT);   
   digitalWrite(19, HIGH);

   delay(2000);
}

    
else if( memcmp(uid,code2,uidLength) == 0 )
{
   Serial.println( "Chip 2" ) ;
   pinMode(16, OUTPUT);   
   digitalWrite(16, LOW);
   pinMode(17, OUTPUT);   
   digitalWrite(17, LOW);
   pinMode(18, OUTPUT);   
   digitalWrite(18, LOW);
   pinMode(19, OUTPUT);   
   digitalWrite(19, HIGH);

   delay(2000);
}

else if( memcmp(uid,code3,uidLength) == 0 )
{
   Serial.println( "Chip 3" ) ;
   pinMode(16, OUTPUT);   
   digitalWrite(16, LOW);
   pinMode(17, OUTPUT);   
   digitalWrite(17, LOW);
   pinMode(18, OUTPUT);   
   digitalWrite(18, LOW);
   pinMode(19, OUTPUT);   
   digitalWrite(19, HIGH);

   delay(2000);
}

else if( memcmp(uid,code4,uidLength) == 0 )
{
   Serial.println( "Chip 4" ) ;
   pinMode(16, OUTPUT);   
   digitalWrite(16, LOW);
   pinMode(17, OUTPUT);   
   digitalWrite(17, LOW);
   pinMode(18, OUTPUT);   
   digitalWrite(18, LOW);
   pinMode(19, OUTPUT);   
   digitalWrite(19, HIGH);

   delay(2000);
}

else if( memcmp(uid,code5,uidLength) == 0 )
{
   Serial.println( "Chip 5" ) ;
   pinMode(16, OUTPUT);   
   digitalWrite(16, LOW);
   pinMode(17, OUTPUT);   
   digitalWrite(17, LOW);
   pinMode(18, OUTPUT);   
   digitalWrite(18, LOW);
   pinMode(19, OUTPUT);   
   digitalWrite(19, HIGH);

   delay(2000);
}
else if( memcmp(uid,code6,uidLength) == 0 )
{
   Serial.println( "Chip 6" ) ;
   pinMode(16, OUTPUT);   
   digitalWrite(16, LOW);
   pinMode(17, OUTPUT);   
   digitalWrite(17, LOW);
   pinMode(18, OUTPUT);   
   digitalWrite(18, LOW);
   pinMode(19, OUTPUT);   
   digitalWrite(19, HIGH);

   delay(2000);
}

else {Serial.println("Falsche UID");
   pinMode(16, OUTPUT);
   digitalWrite(16, HIGH);
   pinMode(17, OUTPUT);
   digitalWrite(17, HIGH);
   pinMode(18, OUTPUT);   
   digitalWrite(18, HIGH);
   pinMode(19, OUTPUT);   
   digitalWrite(19, LOW);

   smtp.debug(1);
   smtp.callback(smtpCallback);
   ESP_Mail_Session session;
   session.server.host_name = SMTP_HOST;
   session.server.port = SMTP_PORT;
   session.login.email = AUTHOR_EMAIL;
   session.login.password = AUTHOR_PASSWORD;
   session.login.user_domain = "";
   SMTP_Message message;
   message.sender.name = "absender@gmx.de";
   message.sender.email = AUTHOR_EMAIL;
   message.subject = "Falsche UID probiert";
   message.addRecipient("", RECIPIENT_EMAIL);
   String htmlMsg = "<div style=\"color:#2f4468;\"><h1>Falscher RFID-Chip erkannt</h1><p>- Sollte das ein Test sein, kann die Email ignoriert werden.</p></div>";
   message.html.content = htmlMsg.c_str();
   message.html.content = htmlMsg.c_str();
   message.text.charSet = "us-ascii";
   message.html.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

    if (!smtp.connect(&session))
    return;
  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Fehler beim versenden der Email, " + smtp.errorReason());
    
       delay(30000);}
    
Serial.println("Nächster Versuch");
  }

  else

  {
   
   pinMode(16, OUTPUT);
   digitalWrite(16, HIGH);
   pinMode(17, OUTPUT);
   digitalWrite(17, HIGH);
   pinMode(18, OUTPUT);   
   digitalWrite(18, HIGH);
   pinMode(19, OUTPUT);   
   digitalWrite(19, HIGH);

  }
  
  int data = digitalRead(Push_button_GPIO);

  if (data != lastbutton_state) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (data != button_state) {
      button_state = data;


      if (button_state == HIGH) {
        LED_state = !LED_state;
      }
    }
  }

  
  digitalWrite(output, LED_state);


  lastbutton_state = data;
}
