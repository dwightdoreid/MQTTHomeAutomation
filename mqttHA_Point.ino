#include <dummy.h>
/*
 

*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

// Update these with values suitable for your network.

String pointID;
char red[5];
const char* ssid = "ssid";
const char* password = "password";
const char* mqttUser = "mqtt User";
const char* mqttPw = "mqtt pw";
IPAddress mqtt_server(192,168,4,800);

WiFiClient espClient;
//PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int lent);
PubSubClient client(mqtt_server, 1883, callback, espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void callback(char* topic, byte* payload, unsigned int lent) {
  client.disconnect();
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < lent; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.print("Message length is: ");
  Serial.println(lent);

  char payloadString[lent];
  if (String(topic)=="newDeviceAdd") {
    for (int i = 0; i < lent; i++) {
      Serial.print((char)payload[i]);
      payloadString[i]=(char)payload[i];
    }
    Serial.println("");
    Serial. println("new device message recvd");
    redEEPromWrite(String(payloadString));
    Serial. println("Point ID written to EEPROM");
    while (1){}
      
  }

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
 reconnect();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void joinControlNet(){
 //
  char pnt[pointID.length()];
  pointID.toCharArray(pnt,pointID.length()+1);
  client.unsubscribe(pnt);
  String payload = "lamp/";
  redEEPromWrite(pointID);
   
  byte payloadBytes[payload.length()];
  payload.getBytes(payloadBytes,payload.length()+1);
  client.publish("newDevice",payloadBytes ,payload.length() );
  Serial.print("Point ID is: ");
  Serial.println(pointID);
  Serial.println(client.subscribe("newDeviceAdd"));
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void redEEPromWrite(String x){
  for (int i=0; i<4;i++){
    EEPROM.write(i, x[i]);
  }
   
  EEPROM.commit();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void redEEPromRead(){
  for (int i=0; i<4;i++){
    red[i] = EEPROM.read(i);
  }
   
  EEPROM.commit();
  pointID=red;
  Serial.print("ID read from EEPROM is: ");
  Serial.println(red);
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "arduinoClient";
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqttUser, mqttPw)) {
      Serial.println("connected");
      char pnt[4];
      pointID.toCharArray(pnt,pointID.length()+1);
      client.subscribe(pnt);
      Serial.print("subscribed to: ");
      Serial.println(pnt);
      client.loop();
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  Serial.println(""); 
  redEEPromRead();
  Serial.println("");
  Serial.print("point ID is: ");
  Serial.println(pointID);
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(0, INPUT);
  reconnect();
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {

  if (!client.connect("arduinoClient", "mqttUser", "mqttuse")) {
    reconnect();
  }
  /*else
  {
     Serial.println("Connected to MQTT server ");
  }*/
  
  client.subscribe("inTopic");
  client.loop();
  
   if (digitalRead(0)==false)
  {
    Serial.println("button pressed");
    joinControlNet();
    delay(250);
  }
}
