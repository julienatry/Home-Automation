#include <Arduino.h>
#include <painlessMesh.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>

#define   MESH_PREFIX     ""
#define   MESH_PASSWORD   ""
#define   MESH_PORT       1234

#define   STATION_SSID     ""
#define   STATION_PASSWORD ""

#define   HOSTNAME ""

void receivedCallback(const uint32_t &from, const String &msg);
void mqttCallback(char* topic, byte* payload, unsigned int length);

IPAddress getlocalIP();

IPAddress myIP(0,0,0,0);
boolean BrokerError = true;


painlessMesh  mesh;
WiFiClient wifiClient;
PubSubClient mqttClient("", 1883, mqttCallback, wifiClient);

void setup() {
  Serial.begin(115200);

  mesh.setDebugMsgTypes(STARTUP);

  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT, WIFI_AP_STA, 11);
  mesh.onReceive(&receivedCallback);

  mesh.stationManual(STATION_SSID, STATION_PASSWORD);
  mesh.setHostname(HOSTNAME);

  mesh.setRoot(true);
  mesh.setContainsRoot(true);
}

void loop() {
  mesh.update();
  mqttClient.loop();

  
  
  if (myIP != getlocalIP()) {
    myIP = getlocalIP();
    Serial.println("Got IP Address");
    if (!mqttClient.connected() ) {
       if (mqttClient.connect("wifiClient", "", "")) {
          BrokerError= false;
          Serial.println("MQTT Broker connected");
          mqttClient.publish("testmqtt","Bridge OK");
          }
          else {
            BrokerError= true;
            Serial.println("MQTT Broker error");
          }
      }
    }
}

void receivedCallback(const uint32_t &from, const String &msg) {
  Serial.printf("bridge: Received from %u msg=%s\n", from, msg.c_str());
  String topic = "painlessMesh/from/" + String(from);
  mqttClient.publish(topic.c_str(), msg.c_str());
}

void mqttCallback(char* topic, uint8_t* payload, unsigned int length) {
  char* cleanPayload = (char*)malloc(length+1);
  payload[length] = '\0';
  memcpy(cleanPayload, payload, length+1);
  String msg = String(cleanPayload);
  free(cleanPayload);

  Serial.println(msg);

  String targetStr = String(topic).substring(16);

  if(targetStr == "gateway")
  {
    if(msg == "getNodes")
    {
      auto nodes = mesh.getNodeList(true);
      String str;
      for (auto &&id : nodes)
        str += String(id) + String(" ");
      mqttClient.publish("painlessMesh/from/gateway", str.c_str());
    }
  }
  else if(targetStr == "broadcast") 
  {
    mesh.sendBroadcast(msg);
  }
  else
  {
    uint32_t target = strtoul(targetStr.c_str(), NULL, 10);
    if(mesh.isConnected(target))
    {
      mesh.sendSingle(target, msg);
    }
    else
    {
      mqttClient.publish("painlessMesh/etatBridge", "0");
    }
  }
}

IPAddress getlocalIP() {
  return IPAddress(mesh.getStationIP());
}