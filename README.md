# HomeIoT

This is a small project to add IoT sensor data to [Home Assistant](https://www.home-assistant.io/) utilising cheap ESP8266 modules

## Features
- Deep sleep so the board does not heat up and effect sensor readings
- MQTT to send data
- Based off wemos dev boards.
- Cases for 3d printing

### Home Assistant setup
For example for a sensor named `ESP_SENSOR_11` add the following to your `configuration.yaml`

```yaml
sensor:
- name: ESP_SENSOR_11 temperature
  platform: mqtt
  device_class: temperature
  state_topic: ESP_SENSOR_11/temperature
  unit_of_measurement: °C
- name: ESP_SENSOR_11 humidity
  platform: mqtt
  device_class: humidity
  state_topic: ESP_SENSOR_11/humidity
 unit_of_measurement: '%'
```

## Temp/Humidity (SHT30)
| Part | Name | Source |
| ---- | ---- | -------|
| Controller | [D1 Mini light](https://wiki.wemos.cc/products:d1:d1_mini_lite) | [$3 AliExpress](https://www.aliexpress.com/item/32795857574.html) |
| Sensor | [SHT30 Shield](https://wiki.wemos.cc/products:d1_mini_shields:sht30_shield) | [$2.3 AliExpress](https://www.aliexpress.com/item/32762136940.html)|

