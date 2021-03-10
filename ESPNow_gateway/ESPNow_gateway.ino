//
// Sample sketch which receives ESPnow data sent by ESPNow sensors and forwards the received data to a MQTT broker
//
// Latest code available on https://github.com/framps/ESP_stuff/tree/main/ESPNow_gateway
// Sensor code is available on https://github.com/framps/ESP_stuff/tree/main/DHT22-BME280-espnow-deepSleep-sensor
//

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

#include <esp_now.h>
#include <WiFi.h>
#include <MQTTClient.h>
#include <jled.h>

#define CHANNEL 1                       // channel of WLAN_AP
#define LED_GREEN 18                    // green LED signals some data received from an ESPnow sensor
#define LED_RED 23                      // red LED signals no connection to home network to forward received data

#define KEEP_ALIVE 90                   // seconds

#define WLAN_AP "Framp-IOT"             // local AP the received data will be sent to, hast to have channel 1
#define WLAN_AP_PWD "Pmarf-IOT"

#define WLAN_GW "Framp-IOT-GW"          // AP created by gateway for sensors, will be channel 1
#define WLAN_GW_PWD "Pmarf-IOT-GW"

#define BROKERIP "192.168.0.42"         // IP of MQTT broker the data will be sent to

union RECEIVED_DATA {
    SENSOR_DATA sensorData;
    uint8_t buffer[sizeof(SENSOR_DATA)];
};

/** MQTT client class to access mqtt broker */
MQTTClient mqttClient(2560);
/** MQTT broker URL */
static const char * mqttBroker = BROKERIP;
/** MQTT connection id */
static const char * mqttID = "Gateway";
/** MQTT user name */
static const char * mqttUser = "";
/** MQTT password */
static const char * mqttPwd = "";
/** WiFi client class to receive messages from mqtt broker */
WiFiClient mqttReceiver;

auto ledGreen = JLed(LED_GREEN).Breathe(2000).DelayAfter(1000);

void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len);

// Init ESP Now with fallback
void InitESPNow() {
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  }
  else {
    Serial.println("ESPNow Init Failed");
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  String Prefix = WLAN_GW;
  String Mac = WiFi.macAddress();
  String SSID = Prefix;
  String Password = WLAN_GW_PWD;
  bool result = WiFi.softAP(SSID.c_str(), Password.c_str(), CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
  }
}

void setup() {
  Serial.begin(115200);

  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);

  digitalWrite(LED_RED, HIGH);       // turn the LED on (HIGH is the voltage level)

  Serial.println("ESPNow Gateway");
  Serial.printf("WLAN_AP: %s - WLAN_GW: %s\n",WLAN_AP,WLAN_GW);

  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(WLAN_AP, WLAN_AP_PWD);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());

  // Start connection to MQTT broker
  // Connect to MQTT broker
  mqttClient.begin(mqttBroker, mqttReceiver);

  Serial.println("Connecting to MQTT broker");

  int connectTimeout = 0;

  while (!mqttClient.connect(mqttID)) {
    delay(100);
    connectTimeout++;
    if (connectTimeout > 10) { // Wait for 1 seconds to connect
      Serial.println("Can't connect to MQTT broker");
      ESP.restart();
    }
  }
  Serial.println("Connected to MQTT");

  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);

  digitalWrite(LED_RED, LOW);    // turn the LED off by making the voltage LOW

}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
  ledGreen.Reset();
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Recv from: "); Serial.println(macStr);

  RECEIVED_DATA rd;
  memcpy(rd.buffer,data,data_len);

  char topic[256];
  sprintf(topic,"sensor/%i","42");
  char bb[256];
  sprintf(bb,"{ \"measurement\": \"%s\", \"id\": \"sensor%i\", \"temperature\": %.1f, \"humidity\": %.1f, \"vcc\": %i }","temperature",rd.sensorData.id,rd.sensorData.temp,rd.sensorData.hum, rd.sensorData.vcc);

  Serial.printf("%s %s\n",topic,bb);
  mqttClient.publish(topic,bb);
}

void loop() {
  if ( ! mqttClient.loop() ) {
    setup();
  }
  ledGreen.Update();
  // Chill
}
