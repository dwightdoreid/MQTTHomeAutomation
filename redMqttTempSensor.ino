/**
 * The MySensors Arduino library handles the wireless radio link and protocol
 * between your home built sensors/actuators and HA controller of choice.
 * The sensors forms a self healing radio network with optional repeaters. Each
 * repeater and gateway builds a routing tables in EEPROM which keeps track of the
 * network topology allowing messages to be routed to nodes.
 *
 * Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
 * Copyright (C) 2013-2015 Sensnology AB
 * Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
 *
 * Documentation: http://www.mysensors.org
 * Support Forum: http://forum.mysensors.org
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 *******************************
 *
 * DESCRIPTION
 *
 * Example sketch showing how to send in DS1820B OneWire temperature readings back to the controller
 * http://www.mysensors.org/build/temp
 */


// Enable debug prints to serial monitor
#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

#define MY_GATEWAY_MQTT_CLIENT
#define MY_GATEWAY_ESP8266

// Set this node's subscribe and publish topic prefix
#define MY_MQTT_PUBLISH_TOPIC_PREFIX "temp_sensor_1-out"
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "temp_sensor_1-in"

// Set MQTT client id
#define MY_MQTT_CLIENT_ID "temp_sensor_1"

// Enable these if your MQTT broker requires usenrame/password
#define MY_MQTT_USER "mqttUser"
#define MY_MQTT_PASSWORD "mqttuser"

// Set WIFI SSID and password
#define MY_ESP8266_SSID "RHN_2"
#define MY_ESP8266_PASSWORD "TronicsWare4eva"

// MQTT broker ip address.
#define MY_CONTROLLER_IP_ADDRESS 192, 168, 0, 166

// The MQTT broker port to to open
#define MY_PORT 1883


#include <SPI.h>
#include <ESP8266WiFi.h>
#include <MySensors.h>  
#include <DallasTemperature.h>
#include <OneWire.h>

//#define COMPARE_TEMP 1 // Send temperature only if changed? 1 = Yes 0 = No

#define ONE_WIRE_BUS D1 // Pin where dallase sensor is connected 
#define MAX_ATTACHED_DS18B20 16
unsigned long SLEEP_TIME = 3000; // Sleep time between reads (in milliseconds)
OneWire oneWire(ONE_WIRE_BUS); // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
DallasTemperature sensors(&oneWire); // Pass the oneWire reference to Dallas Temperature. 
float lastTemperature[MAX_ATTACHED_DS18B20];
int numSensors=2;
bool receivedConfig = false;
bool metric = true;
// Initialize temperature message
MyMessage msg(0,V_TEMP);

void before()
{
  // Startup up the OneWire library
  sensors.begin();
}

void setup()  
{ 
  // requestTemperatures() will not block current thread
  sensors.setWaitForConversion(false);
}

void presentation() {
  // Send the sketch version information to the gateway and Controller
  sendSketchInfo("Temperature Sensor", "1.1");

  // Fetch the number of attached temperature sensors  
  numSensors = sensors.getDeviceCount();

  // Present all sensors to controller
  for (int i=0; i<numSensors && i<MAX_ATTACHED_DS18B20; i++) {   
     present(i, S_TEMP);
  }
}

void loop()     
{     
  // Fetch temperatures from Dallas sensors
  Serial.print("Requesting temperatures...");
  sensors.requestTemperatures();
  Serial.println("Done");
  

  float temperature = sensors.getTempCByIndex(0);
  // Send in the new temperature
  send(msg.setSensor(0).set(temperature,1));
  Serial.print("Temperature is: ");
  Serial.println(temperature);
      
  Serial.println("going to sleep...");
  //sleep(SLEEP_TIME);
  delay(SLEEP_TIME);
  Serial.println("woke up");
}
