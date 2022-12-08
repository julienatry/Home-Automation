#include <painlessMesh.h>
#include <ArduinoJson.h>

#define   MESH_PREFIX     ""
#define   MESH_PASSWORD   ""
#define   MESH_PORT       1234

Scheduler userScheduler;
painlessMesh  mesh;

void sendMessage() ;

Task taskSendMessage(TASK_SECOND * 60 , TASK_FOREVER, &sendMessage);

void sendMessage() {
  mesh.sendBroadcast(mesh.getNodeId());
  taskSendMessage.setInterval(random(TASK_SECOND * 60, TASK_SECOND * 120));
}

void receivedCallback(uint32_t from, String &msg) {
  Serial.println(msg);
}

void newConnectionCallback(uint32_t nodeId) {
}

void changedConnectionCallback() {
}

void nodeTimeAdjustedCallback(int32_t offset) {
}

void setup() {
  Serial.begin(115200);


  mesh.setDebugMsgTypes(STARTUP);

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT , WIFI_AP_STA, 11);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);


  userScheduler.addTask(taskSendMessage);
  taskSendMessage.enable();
}

void loop() {
  userScheduler.execute();
  mesh.update();
}