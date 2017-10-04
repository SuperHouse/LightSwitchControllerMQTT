# Light switch manager

Written by Jonathan Oxer for [SuperHouse Automation Pty Ltd](http://www.superhouse.tv/)  

## Description

Reads digital inputs on an Arduino Mega (or compatible board with on-board
Ethernet, such as a Freetronics EtherMega) to detect button presses.

When a button press is detected, the event is reported to MQTT.

Can optionally display events locally using a Freetronics 128x128 colour OLED module.

See more at http://www.superhouse.tv/

## Required Libraries

1. [PubSubClient.h](https://github.com/knolleary/pubsubclient)

2. [FTOLED.h](https://github.com/freetronics/FTOLED) NOTE: Only required when using the OLED display option.

## Installation

1. Change directory to Arduino's main sketchbook directory
2. git clone https://github.com/superhouse/LightSwitchControllerMQTT.git .
3. Start Arduino IDE.
4. Go to File--->Sketchbook--->LightSwitchControllerMQTT

## Pin Allocations

Assuming a Freetronics EtherMega or EtherDue:

2      DHT temperature / humidity sensor
3      External watchdog "pat" (output)
5      Tilt sensor input (biased high internally)
16-47  Button inputs
48     OLED CS
49     OLED DC
53     OLED reset
54-69  Button inputs

## Button Mappings

Each button is connected to a digital input.

Skips 62 and 64 in east switchboard using EtherDue. I can't remember why!

Port 1: D54-D61 (A0-A7)
Port 2: D62-D69 (A8-A15)
Port 3: D40-D47
Port 4: D16-D23
Port 5: D24-D31
Port 6: D32-D39


## License
    Copyright (C) 2015-2017 SuperHouse Automation Pty Ltd

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
