# Light switch manager

Developed by [SuperHouse Automation Pty Ltd](http://www.superhouse.tv/)  

## Description

Reads digital inputs on an EtherMega or EtherDue to detect button presses.

When a button press is detected, the event is reported to MQTT.

Can optionally display events locally using a Freetronics 128x128 colour OLED module.

See more at http://www.superhouse.tv/

## Required Libraries

1. [PubSubClient.h](https://github.com/knolleary/pubsubclient)

2. [FTOLED.h](https://github.com/freetronics/FTOLED) NOTE: Only required when using the OLED display option.

## Installation

1. Change directory to Arduino's main sketchbook directory
2. git clone https://github.com/superhouse/LightSwitchDigital48MQTT.git .
3. Start Arduino IDE.
4. Go to File--->Sketchbook--->LightSwitchDigital48MQTT

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
