# ESP_stuff

1. LCD_Dimmer: Dim an LCD with SW instead of a potentiometer. Does not work with I2C driven LCD :-(. May work with native LCDs.
2. BlinkNotification: Simple class which allows to create any LED blink patterns for status and error notifications if there is no other way to pass this information to the outside world (e.g. LCD) ([Demovideo](https://www.linux-tips-and-tricks.de/BlinkNotification.mp4))
3. DHT22-BME280-espnow-deepSleep-sensor: Sensor sketch which uses either BME280 or DHT22 sensor and uses this sensor to publish the date via ESPnow to a ESPNow gateway by using deep sleep to minimze power consumption. 
4. ESPNow_Gateway: Sample sketch to receive ESPNow sensor data and forward the data to an MQTT sensor
