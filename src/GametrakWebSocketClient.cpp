#ifndef IS_SERVER

/*
 * GametrakWebSocketClient.cpp
 *
 *  Created on: 02.02.2019
 *
 */

#include "gametrak.h"


#define X_PIN 32
#define Y_PIN 33
#define Z_PIN 34

WiFiMulti wfMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	USE_SERIAL.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			USE_SERIAL.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		USE_SERIAL.printf("%02X ", *src);
		src++;
	}
	USE_SERIAL.printf("\n");
}

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {

	switch(type) {
		case WStype_DISCONNECTED:
			USE_SERIAL.printf("[WSc] Disconnected!\n");
			break;
		case WStype_CONNECTED:
			USE_SERIAL.printf("[WSc] Connected to url: %s\n", payload);

			// send message to server when Connected
			webSocket.sendTXT("Connected");
			break;
		case WStype_TEXT:
			USE_SERIAL.printf("[WSc] get text: %s\n", payload);

			// send message to server
			// webSocket.sendTXT("message here");
			break;
		case WStype_BIN:
			USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
			hexdump(payload, length);

			// send data to server
			// webSocket.sendBIN(payload, length);
			break;
		case WStype_ERROR:			
		case WStype_FRAGMENT_TEXT_START:
		case WStype_FRAGMENT_BIN_START:
		case WStype_FRAGMENT:
		case WStype_FRAGMENT_FIN:
			break;
	}

}

void setup() {
	USE_SERIAL.begin(115200);

  pinMode(X_PIN,INPUT);
  pinMode(Y_PIN,INPUT);
  pinMode(Z_PIN,INPUT);

	//Serial.setDebugOutput(true);
	USE_SERIAL.setDebugOutput(true);

	USE_SERIAL.println();
	USE_SERIAL.println();
	USE_SERIAL.println();

	for(uint8_t t = 4; t > 0; t--) {
		USE_SERIAL.printf("[SETUP] BOOT WAIT %d...\n", t);
		USE_SERIAL.flush();
		delay(1000);
	}

	wfMulti.addAP(SSID, PASSWORD);

	//WiFi.disconnect();
	while(wfMulti.run() != WL_CONNECTED) {
		delay(100);
	}

	// server address, port and URL
//  webSocket.begin("192.168.0.123", 81, "/");
  webSocket.begin("192.168.4.1", 81, "/");

	// event handler
	webSocket.onEvent(webSocketEvent);

	// use HTTP Basic Authorization this is optional remove if not needed
	//webSocket.setAuthorization("user", "Password");

	// try ever 5000 again if connection has failed
	webSocket.setReconnectInterval(5000);

}

void loop() {
	  webSocket.loop();
    int x = analogRead(X_PIN);
    int y = analogRead(Y_PIN);
    int z = analogRead(Z_PIN);

 /*   Serial.print("Analog X: ");
    Serial.print(x);
    Serial.print(" - Y: ");
    Serial.print(y);
 */   Serial.print(" - Z: ");
    Serial.println(z);

    if(x>=300) wsd.sensor.x = map(x,300,4095,-1000,1000); else wsd.sensor.x=-1000;
    if(y<=3700) wsd.sensor.y = map(y,0,3700,-1000,1000); else wsd.sensor.y=1000;
    wsd.sensor.z = map(z,0,4095,1000,0);

    
    Serial.print("Analog X: ");
    Serial.print(wsd.sensor.x);
    Serial.print(" - Y: ");
    Serial.print(wsd.sensor.y);
    Serial.print(" - Z: ");
    Serial.println(wsd.sensor.z);

    webSocket.sendBIN(wsd.bytePacket, sizeof(sensorData_t));
}

#endif
