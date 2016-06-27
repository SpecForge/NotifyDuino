## Introduction

Free Arduino Google Cloud Messaging (GCM) project. It’s simple way to get live message from Arduino board to your Android device - anywhere & anytime!

This project allows to receive messages from Arduino Mega 2560 controller board to custom device, based on Android, through GSM service or user e-mail. 
For this, you will need:
* Arduino Mega 2560 controller board
* ability to appload on controller board the sketch from repository
* device, operated by Android, for example, your smartphone
* download and install the app for Android NotifyDuino.apk


## Project description and work scheme.

The hardware includes controller Arduino Mega 2560 with external ENC28J60 Ethernet module and optionally up to 2 DS18B20 temperature sensors or similar. 
The software - sketch for controller, server to implement controller interacting with GCM service and custom Android application, written in Java.
When changing the logic state of one of four input controller ports or when changing temperature out of valid range, on external event server is sent a command, the server via GCM service sends a caution message to custom Android device or user e-mail.
Through Android app, user can configure network parameters of device, permissible   operating temperature range for each sensor, input signals logic level at which the notification happen, manage logic level of one of controller port or reload the device.
Possible application: overall where it’s necessary to control temperature or inputs controller state, monitor the current states of external devices, sending notifications of dangerous emergency situations, break-ins, remote load control. Since the controller code is open, you can independently add needed functions.

The current scheme of interaction of all project elements is shown in fig.1

![](https://www.specforge.com/media/notifyduino_block_diagram.png)

Figure 1 - block diagram of client-server interaction


1) Android app in the first start passes registration in GCM server and receives from it GCS registration token.
Note: Registration is only possible if your Android device supports Google Services or set Internet connection. 
2) After successful registration, the application will connect to event server and log on it. This server is located at notify.tom.ry (personal!)
3) With occurrence the event on device, it will send data packet to event server, in packet is stored device serial number, notification type and message.
4) Note: For normal operation on device should be properly configured network settings.
5) After receiving notice from device, the vent server will send a message with events description on GCM server, which will send a message to Android app and/or user e-mail, specified in settings.

## The hardware

For device assembling, you will need the next components:
* Arduino Mega 2560 controller board
* ENC28J60 Ethernet module board
* Up to 2 DS18B20, DS18S20, DS1822 temperature sensors

The connection diagram of controller and periphery board is shown in fig.2

![](https://www.specforge.com/media/notifyduino_connection_diagram.png)
Figure 2 - The connection diagram of Arduino Mega 2560 controller board,  ENC28J60 Ethernet module board and temperature sensors

By default is used current writing diagram of ports connection:
```
Input ports: D-18-D21, GND
Output ports: D7, GND
Temperature sensors: A0, A1, 5V, GND
ENC28J60 Ethernet module
```

Arduino Mega 2560  |  ENC28J60 module
-------------------|------------------
       D53         |       CS
-------------------|------------------
	   D51         |       SI
-------------------|------------------
	   D50         |       SO
-------------------|------------------
	   D52         |       SCK
-------------------|------------------
	   3V3         |       VCC
-------------------|------------------
	   GND         |       GND
-------------------|------------------
	   

When connecting temperature sensors to other controller ports or when changing output ports, necessary to make changes in controller firmware source code. Input ports and ENC28J60 Ethernet module connection ports can not be changed.

## Software installation on Arduino Mega 2560 controller board

### Download and install Arduino IDE on the link:
https://www.arduino.cc/en/Main/Software

### Install required libraries (instructions for installing the libraries: https://www.arduino.cc/en/Guide/Libraries), which are located in «libs» folder:
base64 - https://github.com/adamvr/arduino-base64
EEPROM2 - https://github.com/aterentiev/EEPROM2
ENC28J60_ethernet - https://github.com/ntruchsess/arduino_uip
OneWire - https://github.com/pbrook/arduino-onewire
Streaming - https://github.com/kachok/arduino-libraries/tree/master/Streaming
Webduino - https://github.com/sirleech/Webduino
DallasTemperature - https://github.com/milesburton/Arduino-Temperature-Control-Library

### Unique device serial number
When launching sketch at the first time, occurring the generation of random 29-bit serial number. This value is assigned to variable “default_serial_number”. Device serial numbers must not coincide.

### Set the network device parameters.
Open «DSettings.h» file and replace following parameters, actual for your device:
```
default_mac[6] 	    = { 0xFC , 0xC2 , 0x3D , 0x4C , 0x4B , 0x40};
default_gateway[4] 	= { 192, 168, 10, 1 };
default_mask[4] 	= { 255, 255, 255, 0 };
default_dns[4]   	= { 8, 8, 8, 8 };
default_ip[4]	 	= { 192, 168, 10, 113 };
```

After connecting the device to Android app, can be replaced network settings through it.

### 
Changing the device ports order

If you want to use port for input signal different from default, or want to connect temperature sensors to other controller ports, that need to make next changes:
Change in «NotifyDuino.ino» file controller ports:
```
// Temperature and output pins
const int TempSensor1Pin = A0;
const int TempSensor2Pin = A1;
const int outputPin = 7;
```

###
Appload source code into controller board:
- Plug the controller board to PC
- In Arduino IDE choose board model (Tools - Board - Arduino/Genuimo Mega or Mega 2560)
- Open «NotifyDuino.ino» file, and download the firmware in controller, press   Upload button.

## App installation and configuration for Android device

App for Android device, you can download on the link from our site https://specforge.com/downloads/NotifyDuino.apk
Application settings description, you can find on the address
Link to the manual

## Own event server
If you want to set own event server on your side, contact us: info@specforge.com

