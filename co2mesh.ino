#include <SoftwareSerial.h>
#include "MHZ19.h"
#include "painlessMesh.h"

#define MESH_PREFIX     "buhtan"
#define MESH_PASSWORD   "buhtan123"
#define MESH_PORT       5555

SoftwareSerial mySerial(16, 17); // RX, TX
MHZ19 myMHZ19;
painlessMesh mesh;

unsigned long previousMillis = 0;  // переменная для хранения времени предыдущего обновления
const long interval = 2000;        // интервал между отправками данных (в миллисекундах)
int previousCO2 = -1;              // переменная для хранения предыдущего значения CO2

// Callback function for receiving messages
void receivedCallback(uint32_t from, String &msg) {
  Serial.printf("Received from %u msg=%s\n", from, msg.c_str());
}

// Callback for new connections
void newConnectionCallback(uint32_t nodeId) {
  Serial.printf("New Connection, nodeId = %u\n", nodeId);
}

// Callback for dropped connections
void droppedConnectionCallback(uint32_t nodeId) {
  Serial.printf("Dropped Connection, nodeId = %u\n", nodeId);
}

void setup() {
  Serial.begin(115200);
  mySerial.begin(9600);
  myMHZ19.begin(mySerial);

  mesh.setDebugMsgTypes(ERROR | STARTUP | CONNECTION); // Set debug message types
  mesh.init(MESH_PREFIX, MESH_PASSWORD, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onDroppedConnection(&droppedConnectionCallback);
}

void loop() {
  mesh.update(); // Update the mesh network

  unsigned long currentMillis = millis(); // текущее время
  if (currentMillis - previousMillis >= interval) {
    // Сохранить время текущего обновления
    previousMillis = currentMillis;

    int CO2 = myMHZ19.getCO2();
    if (CO2 > 0) { // Проверяем, что данные получены и корректны
      if (CO2 != previousCO2) { // Отправляем данные только при изменении
        Serial.print("CO2 (ppm): ");
        Serial.println(CO2);

        String meshMsg = "04" + String(CO2); // Формируем сообщение для отправки в сеть PainlessMesh
        bool sendStatus = mesh.sendBroadcast(meshMsg); // Отправляем сообщение по сети PainlessMesh
        Serial.print("Send status: ");
        Serial.println(sendStatus ? "Success" : "Failed"); // Печатаем статус отправки

        previousCO2 = CO2; // Обновляем предыдущее значение CO2
      }
    } else {
      Serial.println("Error reading CO2");
    }
  }
}
