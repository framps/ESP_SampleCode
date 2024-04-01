# ESP_SampleCode

1. LCD_Dimmer: Dim an LCD with SW instead of a potentiometer. Does not work with I2C driven LCD :-(. May work with native LCDs.
2. BlinkNotification: Simple class which allows to create any LED blink patterns for status and error notifications if there is no other way to pass this information to the outside world (e.g. LCD) ([Demovideo](https://www.linux-tips-and-tricks.de/BlinkNotification.mp4))
3. ESPNow and DeepSleeep Sample Code
    1. ESPNow_DHT22_BME280_Sensor: Sensor sketch which uses either BME280 or DHT22 sensor and uses this sensor to publish the data via ESPnow and DeepSleep to a ESPNow gateway by using deep sleep to minimze power consumption. Vcc power for the sensor can be turned off to reduce power consumption during deep sleep period. Vcc activation and sensor startup delays are configurable.
    2. ESPNow_gateway: Sample sketch to receive ESPNow sensor data and forward the data to an MQTT broker
4. OWM_BMP280: Sample sketch which queries current Open Weather Map data (temperature, humiditiy and pressure) and measures temperature and pressure with a BMP280.
5. Dewpoint: Calculate dew point, absolute moisture & vapor pressure from temperature & humidity
6. OtaUpdatePOC: Proof of concept for an ESP image update strategy which uses the LastUpdate header field of the image to decide whether an update of the image should be done with OTA.
7. findSensors: Search for active ESP sensors in subnet and display IP address, mac, hostname and an optional description (moved to raspberryTools repository)
8. manageESPs: some scripts to manage your ESPs
   * updateESPs.sh: Update binaries of all ESPs in local net which use EspMQTTClient and it's HTTPWebUpdater method
   * statusESPs.sh: Query status information of ESPs which use ESP8266WebServer to publish some status in JSON

## findSensors.sh

```
findSensors.sh
Scanning subnet 192.168.0.0/24 for ESPs ...

IP address      Mac address       Hostname (Description)
192.168.0.126   a4:cf:12:f5:a4:ef alpha (development room sensor)
192.168.0.123   10:52:1c:5d:5c:9c beta (ESPNow gateway)
192.168.0.101   24:62:ab:f3:04:64 gamma (brightness sensor)
192.168.0.161   a4:cf:12:f4:d9:d4 delta (living room sensor)
192.168.0.165   e0:98:06:86:2a:61 epsilon (IT room sensor)
```
## Sample status information sent by ESPs and queried by statusESPs.sh

```
{
  "bld": {
    "tme": "10:16:20",
    "dte": "Aug  8 2023",
    "file": "DHT22-BME280-sensor.ino"
  },
  "hw": {
    "id": "9",
    "esp": "ESP8266"
  },
  "net": {
    "mac": "A4:CF:12:F4:D9:D4",
    "ip": "192.168.0.161",
    "gw": "192.168.0.1",
    "nm": "255.255.255.0"
  },
  "dev": {
    "lcd": true,
    "bme": true,
    "dht": false
  },
  "misc": {
    "signalStrengh": "-63",
    "vcc": "2979"
  },
  "rt": {
    "connects": "3",
    "startup": "12:30 08.08.2023"
  },
  "chip": {
    "chipId": "16046548",
    "flashChipId": "1458280",
    "flashChipSize": "1048576",
    "freeHeap": "40320"
  }
}
```
