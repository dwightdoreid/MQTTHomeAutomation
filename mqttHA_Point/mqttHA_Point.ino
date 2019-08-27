//#include <dummy.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#define USER_BUTTON 16
#define AP_LED 0
#define WIFI_LED 4
#define SEARCH_LED 5

// Update these with values suitable for your network.
ESP8266WebServer server(80);
IPAddress mqtt_server(192,168,0,100);

String pointID;
char red[5];
const char* mqttUser = "mqttUser";
const char* mqttPw = "mqttuser";
const char* ssid = "test";
const char* passphrase = "testpassword";
String esid;
String epass = "";
String dhcpEnabled = "F";

String st;
String content;
int statusCode;


WiFiClient espClient;
void callback(char* topic, byte* payload, unsigned int lent);
PubSubClient client(mqtt_server, 1883, callback, espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
unsigned long startTime;
long timeHeld;

//Static IP address configuration
IPAddress staticIP(192, 168, 0, 67); //ESP static ip
IPAddress gateway(192, 168, 0, 1);   //IP Address of your WiFi Router (Gateway)
IPAddress subnet(255, 255, 255, 0);  //Subnet mask
IPAddress dns(8, 8, 8, 8);  //DNS
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void connect2Wifi() {

  delay(10);
  //WiFi.begin();

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
  
  Serial.print("DHCP Enabled: ");
  Serial.println(char(EEPROM.read(128)));

  String DHCP_Enabled;
  DHCP_Enabled = char(EEPROM.read(128));
  if (DHCP_Enabled == "T")
  {
     if (wifiConnect())
     {
      Serial.println("");
      Serial.println("Connected to wifi network :/ ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      return;
      } 
  }
  else {
     if (wifiConnectDHCP())
     {
      Serial.println("");
      Serial.println("Connected to wifi network :/ ");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
      return;
      } 
  }
  

}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool wifiConnect(void) {
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
          digitalWrite(SEARCH_LED, LOW);
          digitalWrite(WIFI_LED, HIGH);
          return true; 
        }
        
         
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
} 
/////////////////////////////////////////////////////////////////////////////////////////////////////////
bool wifiConnectDHCP(void) {
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, subnet, gateway, dns);
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
          digitalWrite(SEARCH_LED, LOW);
          digitalWrite(WIFI_LED, HIGH);
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
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
  {
    Serial.println("no networks found");
  }
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
  st = "<select required name='ssid' size='5'>";
  for (int i = 0; i < n; ++i)
    {
      st += "<option value=""></option>";
      // Print SSID and RSSI for each network found
      st += "<option value='";
      st += WiFi.SSID(i);
      st += "'>";
      st += WiFi.SSID(i);
      st += " (RSSI: ";
      st += WiFi.RSSI(i);
      st += ")";
      st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      st += "</option>";
    }
  st += "</select>";
  delay(100);
  WiFi.softAP(ssid, passphrase, 6);
  Serial.println("softap");
  launchWeb(1);
  digitalWrite(SEARCH_LED, LOW);
  digitalWrite(AP_LED, HIGH);
  Serial.println("over");
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void createWebServer(int webtype)
{
  if ( webtype == 1 ) {
    Serial.println("");
    Serial.println("webtype 1 selected");
    server.on("/", []() {
        Serial.println("");
        Serial.println("server received /");
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>";
        content += "<html>";
        content += "<head> <meta charset='utf-8'> <meta http-equiv='X-UA-Compatible' content='IE=edge'> <title></title> <meta name='description' content=''> <meta name='viewport' content='width=device-width, initial-scale=1'> <link rel='stylesheet' href=''> </head>";
        content += "<style>";
        content += ".redInput {display: flex; justify-content: space-between; width:  60%; margin: .5rem;}";
        content += "input[type=submit]{background-color: #4CAF50; border: none; color: white; padding: 16px 32px; text-decoration: none; margin: 4px 2px; cursor: pointer;}";
        content += "</style>";
        content += "<body>";
        content += "<h2>Welcome to redDevice WiFi Setup at IP Address:";
        content += ipStr;
        content += "</h2>";
        content += "<h4>Please select your WiFi network and enter the pass key</h4>";
        content += "<form method='get' action='setting'>";
        content += st;
        content += "<div class='redInput'><label>Key: </label><input type='password' name='pass' length=64></div>";
        content += "<h4>Select IP Address Mode</h4>";
        content += "<div><select name='dhcpEnable'>";
        content += "<option value='T' selected>DHCP</option>";
        content += "<option value='F'>Fixed</option>";
        content += "</select></div>";
        content += "<input type='submit' value='Enter'>";
        content += "</form>";
        content += "</body>";
        content += "</html>";
        server.send(200, "text/html", content);  
    });
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        String qdhcpEnable = server.arg("dhcpEnable");
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
          Serial.println("writing eeprom DHCP enable:"); 
          EEPROM.write(128, qdhcpEnable[0]);
          Serial.print("Wrote: ");
            Serial.println(qdhcpEnable[0]); 
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
    Serial.println("");
    Serial.println("webtype 0 selected");
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
  //pinMode(0, INPUT);
  pinMode(USER_BUTTON, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(WIFI_LED, OUTPUT);
  pinMode(AP_LED, OUTPUT);
  pinMode(SEARCH_LED, OUTPUT);
  digitalWrite(WIFI_LED, LOW);
  digitalWrite(AP_LED, LOW);
  digitalWrite(SEARCH_LED, HIGH);
  digitalWrite(LED_BUILTIN, HIGH); //Turn off
  WiFi.begin();
  delay(1000);
  Serial.println("");
  Serial.println("delay done");
  Serial.println(digitalRead(USER_BUTTON));

  //If button is pressed on power up then start access point for config
  if (digitalRead(USER_BUTTON)==false)
  { 
    Serial.println("button pressed at beginning");
    delay(250);
    if (digitalRead(USER_BUTTON)==false)
      {
        setupAP();
      }
    while(1)
      {
        server.handleClient();
        delay(10);
      }  
  }

  //Try to connect to WiFi
  connect2Wifi();

  //If wifi is not connected start access point for config
  if(!wifiConnect())
  {
    digitalWrite(LED_BUILTIN, LOW); //Turn on
    setupAP();
    while(1)
      {
        server.handleClient();
        delay(10);
      }
  }
  
//  if (digitalRead(4)==false)
//  { 
//    Serial.println("button pressed at beginning");
//    delay(250);
//    if (digitalRead(4)==false)
//      {
//        setupAP();
//      }
//    while(1)
//      {
//        server.handleClient();
//        delay(10);
//      }
//    
//  }
//  else
//  {
//    Serial.println(""); 
//    redEEPromRead();
//    Serial.println("");
//    Serial.print("point ID is: ");
//    Serial.println(pointID);
//    pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output  
//    connect2Wifi();
//    client.setServer(mqtt_server, 1883);
//    client.setCallback(callback);
//    pinMode(0, INPUT);
//    delay(1000);
//    reconnect();
//   }
  
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
//  setupAP();

//  if (!client.connected() && (WiFi.status() == WL_CONNECTED)) {
//    Serial.println("rwwwiaiau");
//    reconnect();
//  }
// 
//  client.loop();
//  
//   if (digitalRead(0)==false)
//  {
//    Serial.println("button pressed");
//    delay(250);
//     if (digitalRead(0)==false)joinControlNet();
//    
//  }

    Serial.println("looping");
    delay(1000);
}
