#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>       
#include <NTPClient.h>
#include <PubSubClient.h>
#include <WEMOS_SHT3X.h>
#include <Timer.h>

/*
Using the following libraries:

  * https://github.com/tzapu/WiFiManager
  * https://github.com/arduino-libraries/NTPClient
  * https://github.com/knolleary/pubsubclient
  * https://github.com/wemos/WEMOS_SHT3x_Arduino_Library
  * https://github.com/JChristensen/Timer
*/

#define NODE_NAME "esp8266#1"

// Timer
//
Timer timer;

// SHT30
//
SHT3X sht30(0x45);

// ESP8266
//
//WiFiClientSecure ESPClient;
WiFiClient ESPClient;
const char *WIFIAPName = "ESP01";
const char *WIFIAPPassword = "password";

// NTP
//
WiFiUDP ntpUDP;
NTPClient NTPClient(ntpUDP, "au.pool.ntp.org", 3600, 60000);

// MQTT
//
const char *MQTTServer = "mosquitto.server";
uint16_t MQTTPort = 1883;
const char *MQTTUser = "user";
const char *MQTTPassword = "password";
const char *MQTTBaseTopic = "esp8266/1";
const char *MQTTBaseTopicAll = "esp8266/1/#";

void MQTTCallback(char* topic, byte* payload, unsigned int length) {
  char message[1024];

  for (int i = 0; i < length; i++) { message[i] = (char)payload[i]; }
  message[length] = '\0';

  Serial.print("MQTTCallback(): topic=[");
  Serial.print(topic);
  Serial.print("], message=[");
  Serial.print(message);
  Serial.println("]");
}

PubSubClient MQTTClient(MQTTServer, MQTTPort, MQTTCallback, ESPClient);

/*--------------------------------------------------------------------------------------------------*/

void setupSerial() {
  Serial.begin(115200);
}

void setupTimer() {
  timer.every(5000, sendUpdates);
}

void setupWIFI() {
  Serial.println("setupWIFI():");
  WiFiManager wifiManager;
  wifiManager.autoConnect(WIFIAPName, WIFIAPPassword);
}

void setupLEDs() {
  pinMode(LED_BUILTIN, OUTPUT);
  LEDOff(LED_BUILTIN);
}

/*--------------------------------------------------------------------------------------------------*/

String getTimeViaNTP() {
  NTPClient.update();
  String currentTime = NTPClient.getFormattedTime();
  Serial.println(currentTime);

  return currentTime;
}

void LEDOn(int pin) {
  digitalWrite(pin, LOW);
}

void LEDOff(int pin) {
  digitalWrite(pin, HIGH);
}

void UserLEDOn() {
  LEDOn(LED_BUILTIN);
}

void UserLEDOff() {
  LEDOff(LED_BUILTIN);
}

float getTemperature() {
  sht30.get();
  return sht30.cTemp;
}

void sendUpdates() {
  publishTemperature(getTemperature());
}

void MQTTPublish(char *topic, char *message, bool retained=true) {
  char t[64];

  sprintf(t, "%s/%s", MQTTBaseTopic, topic);

  MQTTClient.publish(t, message, retained);
}

void MQTTPublishPing() {
  char msg[100];

  sprintf(msg, "%s UP", NODE_NAME);
  MQTTPublish("ping", msg, false);
}

void MQTTSubscribe() {
  MQTTClient.subscribe(MQTTBaseTopicAll);
}

void MQTTReconnect() {
  while (!MQTTClient.connected()) {
    Serial.println("MQTTReconnect(): Connecting..");
    if (MQTTClient.connect("esp8266#1", MQTTUser, MQTTPassword, 0, 0, true, 0)) {
      Serial.println("MQTTReconnect(): Connected!");
      MQTTPublishPing();
      MQTTSubscribe();
    } else {
      Serial.print("MQTTReconnect(): Failed, rc=");
      Serial.print(MQTTClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void publishTemperature(float in) {
  char value[8];
  dtostrf(in, 0, 2, value);
  Serial.print("publishTemperature(): value=[");
  Serial.print(value);
  Serial.println("]");
  MQTTPublish("temperature", value);
}

/*--------------------------------------------------------------------------------------------------*/

void setup() {
  setupSerial();
  setupLEDs();
  setupWIFI();
  getTimeViaNTP();
}

void loop() {
  timer.update();

  if (!MQTTClient.connected()) { MQTTReconnect(); }
  MQTTClient.loop();
}
