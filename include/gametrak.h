#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WebSocketsClient.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <driver/dac.h>
//#include <ArduinoJson.h>
#include <WebSocketsServer.h>

#define SSID "ESP32Server"
#define PASSWORD "87654321"
#define SERVER_PORT 5000

typedef struct sensorData_t{
  int x;
  int y;
  int z;
};

typedef union Websock_Packet_t{
    sensorData_t sensor;
    byte bytePacket[sizeof(sensorData_t)];
};

Websock_Packet_t wsd;  
