/**
 Light switch reader using IO Breakout shield with RJ45 input breakouts
 
 Copyright 2015 SuperHouse Automation Pty Ltd <info@superhouse.tv>

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
 */
/*--------------------------- Configuration ------------------------------*/
/* Network config */
#define ENABLE_DHCP                 true   // true/false
#define ENABLE_MAC_ADDRESS_ROM      true   // true/false
#define MAC_I2C_ADDRESS             0x50   // Microchip 24AA125E48 I2C ROM address
static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };  // Set if no MAC ROM
static uint8_t ip[] = { 192, 168, 1, 35 }; // Use if DHCP disabled

/* Display config */
#define ENABLE_OLED                 false  // true/false. Enable if you have a Freetronics OLED128 connected
const long oled_timeout =            20;   // Seconds before screen blanks after last activity

// Panel-specific configuration:
//int panelId = 3; // Bathroom
//int panelId = 5; // Amelia's room
//int panelId = 6; // Thomas' room
//int panelId = 7; // Hall West
//int panelId = 8; // Hall East
//int panelId = 9; // Master bedroom
//int panelId = 10; // Doorbell / RFID scanner
//int panelId = 11;  // Lounge
//int panelId = 12;  // Garage SE
int panelId = 13;  // East switchboard
//int panelId = 14;  // New West switchboard

/*------------------------------------------------------------------------*/

/* Required for networking */
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>

/* Required for OLED */
#include "Wire.h"
#include <SD.h>
#include <FTOLED.h>
#include <fonts/SystemFont5x7.h>

/* Required for temperature / humidity sensor */
#include "DHT.h"

/* Use 128x128 pixel OLED module ("OLED128") to display status */
const byte pin_cs     = 48;
const byte pin_dc     = 49;
const byte pin_reset  = 53;
OLED oled(pin_cs, pin_dc, pin_reset);
OLED_TextBox box(oled);
long lastActivityTime = 0;

/* Watchdog timer setup */
#define WDT_PIN 3

/* MQTT setup */
IPAddress server(192,168,1,111);        // MQTT broker
char humidTopic[30] = "/lightswitcheast/humidity";    // MQTT topic
char tempTopic[30]  = "/lightswitcheast/temperature"; // MQTT topic
char messageBuffer[100];
char topicBuffer[100];
char clientBuffer[50];

/* Humidity sensor settings */
#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
unsigned long timeLater = 0;

/**
 * MQTT callback
 */
void callback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Instantiate MQTT client
//PubSubClient client(server, 1883, callback);
EthernetClient ethclient;
PubSubClient client(ethclient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    String clientString = "Reconnecting Arduino-" + String(Ethernet.localIP());
    clientString.toCharArray(clientBuffer, clientString.length()+1);
    if (client.connect(clientBuffer)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //String clientString = "Starting up Arduino-" + String(Ethernet.localIP());
      //String clientString = "Starting up Arduino-" + Ethernet.localIP();
      clientString.toCharArray(clientBuffer, clientString.length() + 1);
      client.publish("events", clientBuffer);
      // ... and resubscribe
      //client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/* ************************************************************************************* */
/* Button setup */
static byte lastButtonState[48] = {   0,  0,  0,  0,    0,  0,  0,  0,
                                      0,  0,  0,  0,    0,  0,  0,  0,
                                      0,  0,  0,  0,    0,  0,  0,  0,
                                      0,  0,  0,  0,    0,  0,  0,  0,
                                      0,  0,  0,  0,    0,  0,  0,  0,
                                      0,  0,  0,  0,    0,  0,  0,  0 };
static byte buttonArray[48]     = {  54, 55, 56, 57,   58, 59, 60, 61,
                                     62, 63, 64, 65,   66, 67, 68, 69,
                                     40, 41, 42, 43,   44, 45, 46, 47,
                                     16, 17, 18, 19,   20, 21, 22, 23,
                                     24, 25, 26, 27,   28, 29, 30, 31,
                                     32, 33, 34, 35,   36, 37, 38, 39 };

byte lastButtonPressed         = 0;
#define DEBOUNCE_DELAY 50
/* ************************************************************************************* */

/**
 * Initial configuration
 */
void setup()
{
  pinMode(WDT_PIN, OUTPUT);
  digitalWrite(WDT_PIN, LOW);
  
  if( ENABLE_OLED == true )
  {
    oled.begin();
    oled.selectFont(SystemFont5x7);
    box.setForegroundColour(DODGERBLUE);
    
    box.println(F("     SuperHouse.TV      "));
    box.println(F("   Button Sensor v2.0   "));
    box.println(F("Getting MAC address:    "));
    box.print  (F("  "));
  }
  
  Wire.begin(); // Wake up I2C bus
  Serial.begin(9600);  // Use the serial port to report back readings
  
  if( ENABLE_MAC_ADDRESS_ROM == true )
  {
    Serial.print(F("Getting MAC address from ROM: "));
    mac[0] = readRegister(0xFA);
    mac[1] = readRegister(0xFB);
    mac[2] = readRegister(0xFC);
    mac[3] = readRegister(0xFD);
    mac[4] = readRegister(0xFE);
    mac[5] = readRegister(0xFF);
  } else {
    Serial.print(F("Using static MAC address: "));
  }
  // Print the IP address
  char tmpBuf[17];
  sprintf(tmpBuf, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println(tmpBuf);
  
  if( ENABLE_OLED == true )
  {
    box.println(tmpBuf);
  }
  
  // setup the Ethernet library to talk to the Wiznet board
  if( ENABLE_DHCP == true )
  {
    Ethernet.begin(mac);      // Use DHCP
  } else {
    Ethernet.begin(mac, ip);  // Use static address defined above
  }
  
  // Print IP address:
  Serial.print(F("My IP: http://"));
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    if( thisByte < 3 )
    {
      Serial.print(".");
    }
  }
  
  
  Serial.println();
  Serial.println(Ethernet.localIP());
  
  if( ENABLE_OLED == true )
  {
      box.println(F("My IP:"));
      box.print("  ");
      for (byte thisByte = 0; thisByte < 4; thisByte++) {
      // print the value of each byte of the IP address:
      box.print(Ethernet.localIP()[thisByte], DEC);
      if( thisByte < 3 )
      {
        box.print(".");
      }
    }
    box.println();
  }
  
  Serial.println("Setting input pull-ups");
  for( byte i = 0; i < 48; i++)
  {
    if((buttonArray[i] != 8)
    && (buttonArray[i] != 10 ))
    {
      pinMode(buttonArray[i], INPUT_PULLUP);
      Serial.print(buttonArray[i]);
      Serial.print(" ");
    }
  }
  Serial.println();

  /* Connect to MQTT broker */
  Serial.println("connecting...");
  client.setServer(server, 1883);
  client.setCallback(callback);
  String clientString = "Starting Arduino-" + Ethernet.localIP();
  clientString.toCharArray(clientBuffer, clientString.length() + 1);
  client.publish("events", clientBuffer);
  
  Serial.println("Ready.");
}


/**
 * Main program loop
 */
void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  
  if( ENABLE_OLED == true )
  {
    if( millis() > (lastActivityTime + (1000 * oled_timeout)))
    {
      oled.setDisplayOn(false);
    }
  }

  unsigned long timeNow = millis();
  if (timeNow >= timeLater) {
    timeLater = timeNow + 60000;

    float humidity    = dht.readHumidity();
    float temperature = dht.readTemperature();

    // Below from stackxchange.com
    char tempC[10];
    dtostrf(temperature,1,2,tempC);
    char relH[10];
    dtostrf(humidity,1,2,relH);

    client.publish(tempTopic, tempC);
    if(client.publish(humidTopic, relH))
    {
      resetWatchdog();
    }

    Serial.print("T: ");
    Serial.print(temperature, DEC);
    Serial.print("C H:");
    Serial.print(humidity, DEC);
    Serial.println("%");
  }
  
  client.loop();
  
  byte i;
  for( i = 0; i < 48; i++) {
    processButtonDigital( i );
  }
}


/**
 */
void processButtonDigital( byte buttonId )
{
  if(  (buttonArray[buttonId] != 8)    // Damn! I don't remember why these exceptions are here :-( Should have made a comment
    && (buttonArray[buttonId] != 10)
  )
  {
    int sensorReading = digitalRead( buttonArray[buttonId] );
    //Serial.print(buttonId, DEC);
    //Serial.print(": ");
    //Serial.println(sensorReading, DEC);
    
    if( sensorReading == 0 )  // Input pulled low to GND. Button pressed.
    {
      //Serial.println( "Button pressed" );
      if( lastButtonState[buttonId] == 0 )
      {
        if((millis() - lastActivityTime) > DEBOUNCE_DELAY)
        {
          lastActivityTime = millis();
          if( ENABLE_OLED == true )
          {
            oled.setDisplayOn(true);
          }
    
          lastButtonPressed = buttonId;
          Serial.print( "transition on ");
          Serial.print( buttonId, DEC );
          Serial.print(" (input ");
          Serial.print( buttonArray[buttonId] );
          Serial.println(")");
        
          String messageString = String(panelId) + "-" + String(buttonArray[buttonId]);
          messageString.toCharArray(messageBuffer, messageString.length()+1);
        
          //String topicString = "device/" + String(panelId) + "/button";
          String topicString = "buttons";
          topicString.toCharArray(topicBuffer, topicString.length()+1);
  
          //client.publish(topicBuffer, messageBuffer);
        
          client.publish("buttons", messageBuffer);
          //client.publish("buttons", "button pressed");
          if( ENABLE_OLED == true )
          {
            box.setForegroundColour(LIMEGREEN);
            box.print(F("Button pressed: "));
            box.println(buttonId);
          }
        }
      } else {
        // Transition off
        //digitalWrite(statusArray[buttonId-1], LOW);
        //digitalWrite(13, LOW);
      }
      lastButtonState[buttonId] = 1;
    }
    else {
      lastButtonState[buttonId] = 0;
    }
  }
}



/**
 * Required to read the MAC address ROM
 */
byte readRegister(byte r)
{
  unsigned char v;
  Wire.beginTransmission(MAC_I2C_ADDRESS);
  Wire.write(r);  // Register to read
  Wire.endTransmission();

  Wire.requestFrom(MAC_I2C_ADDRESS, 1); // Read a byte
  while(!Wire.available())
  {
    // Wait
  }
  v = Wire.read();
  return v;
}

/**
 * Pat the watchdog so it won't reset us. WDT set to 5 minutes so it
 * must be exercised more frequently than that.
 */
void resetWatchdog() {
  digitalWrite(WDT_PIN, HIGH);
  delay(20);
  digitalWrite(WDT_PIN, LOW);
}
