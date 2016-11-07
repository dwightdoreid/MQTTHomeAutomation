#include <dummy.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

// Update these with values suitable for your network.

String pointID;
char red[5];
const char* mqttUser = "mqttUser";
const char* mqttPw = "mqttuser";
const char* ssid = "test";
const char* passphrase = "testpassword1";
String esid;
String epass = "";

String st;
String content;
int statusCode;
ESP8266WebServer server(80);
IPAddress mqtt_server(192,168,0,100);

WiFiClient espClient;
//PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int lent);
PubSubClient client(mqtt_server, 1883, callback, espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
unsigned long startTime;
long timeHeld;
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void setup_wifi() {

  delay(10);
  WiFi.begin();

  Serial.println("Reading EEPROM ssid");
  
  for (int i = 32; i < 64; ++i)
    {
      esid += char(EEPROM.read(i));
    }
  Serial.print("SSID: ");
  Serial.println(esid);
  Serial.println("Reading EEPROM pass");
  
  for (int i = 64; i < 128; ++i)
    {
      epass += char(EEPROM.read(i));
    }
  Serial.print("PASS: ");
  Serial.println(epass);  
         if (testWifi()) {
          Serial.println("");
          Serial.println("Connected to wifi network :/ ");
          launchWeb(0);
          return;
        } 
  //}
  setupAP();
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool testWifi(void) {
  WiFi.mode(WIFI_STA);
  int c = 0;
  if ( esid.length() > 1 ) {
        Serial.println("Trying to connect to wifi net..");
        WiFi.begin(esid.c_str(), epass.c_str());
        while (WiFi.status() != WL_CONNECTED&&c<20) {
          delay(500);
          Serial.print(".");
          c++;
        }
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("Connected to wifi network :/ ");
          return true; 
        }
        
         
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started"); 
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void setupAP(void) {
  client.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     }
  }
  Serial.println(""); 
  st = "<ol>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<li>";
      st += WiFi.SSID(i);
      st += " (";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</li>";
    }
  st += "</ol>";
  delay(100);
  //WiFi.softAP(ssid, passphrase, 6);
  WiFi.softAP(ssid);
  Serial.println("softap");
  launchWeb(1);
  Serial.println("over");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void createWebServer(int webtype)
{
  if ( webtype == 1 ) {
    Serial.println("");
    Serial.println("webtype 1 selected");
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>Hello from ESP8266 at ";
        content += ipStr;
        content += "<p>";
        content += st;
        content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit'></form>";
        content += "</html>";
        server.send(200, "text/html", content);  
    });
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 0; i < 128; ++i) { EEPROM.write(i, 0); }
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
            
          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(32+i, qsid[i]);
              Serial.print("Wrote: ");
              Serial.println(qsid[i]); 
            }
          Serial.println("writing eeprom pass:"); 
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(64+i, qpass[i]);
              Serial.print("Wrote: ");
              Serial.println(qpass[i]); 
            }    
          EEPROM.commit();
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) {
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      server.send(200, "application/json", "{\"IP\":\"" + ipStr + "\"}");
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 128; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
    });
  }
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
  Serial.println("Reconnecting MQTT...");
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "arduinoClient";
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqttUser, mqttPw)) {
      Serial.println("connected");
      char pnt[4];
      if (pointID.length()==0){
        pointID = "Test";
      }
      else{
        pointID.toCharArray(pnt,pointID.length()+1);
        client.subscribe(pnt);
      }
      
      Serial.print("subscribed to: ");
      Serial.println(pnt);
      client.loop();
      Serial.print("MQTT Connection status:  ");
      Serial.println(client.connected());
      delay(500);
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

//  if (!client.connect("arduinoClient", "mqttUser", "mqttuse")) {
//    reconnect();
//  }
  if (!client.connected() && (WiFi.status() == WL_CONNECTED)) {
    Serial.println("rwwwiaiau");
    reconnect();
  }
  /*else
  {
     Serial.println("Connected to MQTT server ");
  }*/
  
  client.subscribe("inTopic");
  client.loop();
  
//   if (digitalRead(0)==false)
//  {
//    Serial.println("button pressed");
//    joinControlNet();
//    delay(250);
//  }

   if (digitalRead(0)==false){
    startTime=millis();  
    while (digitalRead(0)==false){
        timeHeld=millis()-startTime;
        Serial.print("Timeheld is: ");
        Serial.println(timeHeld);        
     }    
  }
  if (timeHeld>1000) {Serial.println("time is long time");setupAP();}
  if (timeHeld>50 && timeHeld<750) {Serial.println("time is good");joinControlNet();}
  timeHeld=0;
   
}
