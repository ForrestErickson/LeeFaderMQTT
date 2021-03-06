/* LeeFaderMQTT
 * Lee Fader's code 20210607
 * MQTT test code
 * Processor ESP32
 *  
 *  Originaly inspired by code at: https://github.com/VeeruSubbuAmi/v2.0-Temperature-Data-record-on-AWS-IoT-Core-with-ESP32
 */

//This code is bing thrashed uppon to try to investigate problems where the ESP32 did not go to sleep
//20210607 Evidence that the MQTT client / broker connection failed.
//Modified by Forrest Erickson for finite WiFi station connection attempts
//Modified by Forrest Erickson for finite MQTT client connection attempts



#include "SPIFFS.h"
#include <WiFiClientSecure.h>
#include <Wire.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>
//#include <TimeLib.h>

RTC_DATA_ATTR int bootCount = 0;
/*
const int potPin_A = 34;
*/
int potValue_A = 0;


//const char* ssid = "XXXXXXXXX";                                     //Provide your SSID
//const char* password = "XXXXXXXXX";                                          // Provide Password
//const char* mqtt_server = "XXXXXXXXXXXXXX.amazonaws.com"; // Relace with your MQTT END point
//const int mqtt_port = 8883;

//WiFi user and PW and for for lab MQTT server at Forrest Lee Erickson
const char* ssid     = "NETGEAR_11N";     // Netgear WAC104 SN: 4SL373BC00087
const char* password = "Heavybox201";  // Lab wifi router
//const char* password = "Heavybox202";  // bad pw.
const char* mqtt_server = "192.168.1.78"; // Relace with your MQTT END point
const int mqtt_port = 8883;

String Read_rootca;
String Read_cert;
String Read_privatekey;
//=============================================================================================================================
#define BUFFER_LEN  256
long lastMsg = 0;
char msg[BUFFER_LEN];
int value = 0;
byte mac[6];
char mac_Id[18];
int count = 1;
//=============================================================================================================================

WiFiClientSecure espClient;
PubSubClient client(espClient);


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Loop until we're WiFi connected but only a maximum number of times
  const int WIFI_MAX_ATTEMPTS = 5;
  int wifiAttemptCount =0;
  while ((WiFi.status() != WL_CONNECTED) & wifiAttemptCount++< WIFI_MAX_ATTEMPTS ) {
    delay(500);
    Serial.print(".");
  }// end != WL_CONNECTED
  Serial.print("wifiAttemptCount: ");
  Serial.println(wifiAttemptCount);

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
void reconnect() {
  // Loop until we're reconnected but only a maximum number of times
  const int MQTT_MAX_ATTEMPTS = 5;
  int mqttAttemptCount =0;
  while (!client.connected() & (mqttAttemptCount++ < MQTT_MAX_ATTEMPTS ) ) {
  //while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("ei_out", "hello world");
      // ... and resubscribe
      client.subscribe("Blue_in");
digitalWrite(18, LOW);    // turn the LED ON by making the voltage HIGH

//=========== SENSOR READING CODE GOES HERE ========================================
  
  potValue_A = 2000;
  delay(500);

  
  Serial.print("probe A; ");
  Serial.print(potValue_A);

    
  delay(500);

StaticJsonDocument<200> doc;
  doc["soil_0"] = potValue_A;

  char buffer[500];
  serializeJson(doc, buffer);
  Serial.println(buffer);
  client.publish("ESP32_Single", buffer);

//============ END SENSOR CODE ===================================

      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }// end while (!client.connected()
  Serial.print("mqttAttemptCount: ");
  Serial.println(mqttAttemptCount);
}//end reconnect

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);
  pinMode(18, OUTPUT);
  Serial.setDebugOutput(true);

  digitalWrite(16, LOW);    // turn the LED off 
  digitalWrite(17, HIGH);    // turn the LED off 
  digitalWrite(18, HIGH);    // turn the LED ON 

  setup_wifi();
  delay(1000);
  digitalWrite(17, LOW);    // turn the LED on 
  //=============================================================
  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  //=======================================
  //Root CA File Reading.
  File file2 = SPIFFS.open("/AmazonRootCA1.pem", "r");
  if (!file2) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("Root CA File Content:");
  while (file2.available()) {
    Read_rootca = file2.readString();
    Serial.println(Read_rootca);
  }
  //=============================================
  // Cert file reading
  File file4 = SPIFFS.open("/xxxxxxxx-certificate.pem.crt", "r");
  if (!file4) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("Cert File Content:");
  while (file4.available()) {
    Read_cert = file4.readString();
    Serial.println(Read_cert);
  }
  //=================================================
  //Privatekey file reading
  File file6 = SPIFFS.open("/xxxxxxxx-private.pem.key", "r");
  if (!file6) {
    Serial.println("Failed to open file for reading");
    return;
  }
  Serial.println("privateKey File Content:");
  while (file6.available()) {
    Read_privatekey = file6.readString();
    Serial.println(Read_privatekey);
  }
  //=====================================================

  char* pRead_rootca;
  pRead_rootca = (char *)malloc(sizeof(char) * (Read_rootca.length() + 1));
  strcpy(pRead_rootca, Read_rootca.c_str());

  char* pRead_cert;
  pRead_cert = (char *)malloc(sizeof(char) * (Read_cert.length() + 1));
  strcpy(pRead_cert, Read_cert.c_str());

  char* pRead_privatekey;
  pRead_privatekey = (char *)malloc(sizeof(char) * (Read_privatekey.length() + 1));
  strcpy(pRead_privatekey, Read_privatekey.c_str());

  Serial.println("================================================================================================");
  Serial.println("Certificates that passing to espClient Method");
  Serial.println();
  Serial.println("Root CA:");
  Serial.write(pRead_rootca);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("Cert:");
  Serial.write(pRead_cert);
  Serial.println("================================================================================================");
  Serial.println();
  Serial.println("privateKey:");
  Serial.write(pRead_privatekey);
  Serial.println("================================================================================================");

  espClient.setCACert(pRead_rootca);
  espClient.setCertificate(pRead_cert);
  espClient.setPrivateKey(pRead_privatekey);

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //====================================================================================================================
  WiFi.macAddress(mac);
  snprintf(mac_Id, sizeof(mac_Id), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print("mac_Id: ");
  Serial.println(mac_Id);
  //=====================================================================================================================

    if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(500);

  esp_sleep_enable_timer_wakeup(5000000);

  //Go to sleep now
  esp_deep_sleep_start();

}


void loop() {



}
//xxxxxxxxxxxxxx.amazonaws.com
//xxxxxxxxxxxxxx.amazonaws.com
