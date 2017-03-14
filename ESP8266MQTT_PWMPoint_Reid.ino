
// Enable debug prints to serial monitor
//#define MY_DEBUG

// Use a bit lower baudrate for serial prints on ESP8266 than default in MyConfig.h
#define MY_BAUD_RATE 9600

#define MY_GATEWAY_MQTT_CLIENT
#define MY_GATEWAY_ESP8266

// Set this node's subscribe and publish topic prefix
#define MY_MQTT_PUBLISH_TOPIC_PREFIX "mygateway1-out"
#define MY_MQTT_SUBSCRIBE_TOPIC_PREFIX "mygateway1-in"

// Set MQTT client id
#define MY_MQTT_CLIENT_ID "mysensors-1"

// Enable these if your MQTT broker requires usenrame/password
#define MY_MQTT_USER "mqttUser"
#define MY_MQTT_PASSWORD "mqttuser"

// Set WIFI SSID and password
#define MY_ESP8266_SSID "ReidMQTT"
#define MY_ESP8266_PASSWORD "mqttuser"



// MQTT broker ip address.
#define MY_CONTROLLER_IP_ADDRESS 192, 168, 2, 10

// The MQTT broker port to to open
#define MY_PORT 1883


#include <ESP8266WiFi.h>
#include <MySensors.h>

#define LED_PIN 15  // Arduino Digital I/O pin number for first relay (second on pin+1 etc)


void setup()
{
  pinMode(LED_PIN, OUTPUT);
  analogWrite(LED_PIN, 100);
}

void presentation()
{
	// Present locally attached sensors here
}


void loop()
{
	// Send locally attech sensors data here
}

void receive(const MyMessage &message)
{
    int requestedLevel = atoi( message.data );
    Serial.print("Requested Level is: ");
    Serial.println(requestedLevel);    
    analogWrite(LED_PIN, requestedLevel); 
}
