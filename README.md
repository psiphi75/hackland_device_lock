# Hackland Device Lock

This repository contains code that will run on a NodeMCU (ESP8266). It can be used as a lock/unlock mechanism. It uses
the MQTT protocol via Wifi.

This code can be executed on any ESP8266 / ESP32 board, the only required interface is WiFi and connection to internet.

## How to install the compile toolchain

You can follow the instructions here: https://github.com/espressif/ESP8266_RTOS_SDK

## How to congifure this project

Run `make menuconfig` then go to the "HACKLAND Lock Configuration" section and configure this for your set up. The
"Broker URL" needs to be the IP address of the MQTT broker.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```sh
make -j4 flash monitor
```

(To exit the serial monitor, type `Ctrl-]`.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## The MQTT protocol

_Registration_

When the device first boots and has connected to WiFi and the MQTT Broker it will send "`LOCK`" to the `/device/[MAC Address]/register` topic. Where `MAC Address` is the unique ID of the ESP device.

This means that a device can be registered and configured via the central server. This is more convient than compiling the code
for each device individually.

_Locking_

By default the device will be locked. When a numeric value is sent to the `/device/[MAC Address]/unlock` topic, the device
will unlock for the given amount of time (in seconds).
