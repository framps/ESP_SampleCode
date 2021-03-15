# ESP_stuff

1. LCD_Dimmer: Dim an LCD with SW instead of a potentiometer. Does not work with I2C driven LCD :-(. May work with native LCDs.
2. BlinkNotification: Simple class which allows to create any LED blink patterns for status and error notifications if there is no other way to pass this information to the outside world (e.g. LCD) ([Demovideo](https://www.linux-tips-and-tricks.de/BlinkNotification.mp4))
3. ESPNow_DHT22_BME280_Sensor: Sensor sketch which uses either BME280 or DHT22 sensor and uses this sensor to publish the data via ESPnow to a ESPNow gateway by using deep sleep to minimze power consumption. Vcc power can be turned on and off to reduce power consumption. Vcc activation and sensor startup delays are configurable.
4. ESPNow_gateway: Sample sketch to receive ESPNow sensor data and forward the data to an MQTT sensor
