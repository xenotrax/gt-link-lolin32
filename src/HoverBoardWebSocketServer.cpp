#ifdef IS_SERVER

/*
 * HoverBoardWebSocketServer.ino
 *
 *  Created on: 02.02.2019
 *
 */

#include "gametrak.h"

#define RX1_pin 16
#define TX1_pin 15

WiFiServer server(SERVER_PORT);
WebSocketsServer webSocket = WebSocketsServer(81);

#define Serial Serial

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16) {
	const uint8_t* src = (const uint8_t*) mem;
	Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
	for(uint32_t i = 0; i < len; i++) {
		if(i % cols == 0) {
			Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
		}
		Serial.printf("%02X ", *src);
		src++;
	}
	Serial.printf("\n");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

    switch(type) {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
            {
                IPAddress ip = webSocket.remoteIP(num);
                Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

				// send message to client
				webSocket.sendTXT(num, "Connected");
            }
            break;
        case WStype_TEXT:
            Serial.printf("[%u] get Text: %s\n", num, payload);

            // send message to client
            // webSocket.sendTXT(num, "message here");

            // send data to all connected clients
            // webSocket.broadcastTXT("message here");
            break;
        case WStype_BIN:
            //Serial.printf("[%u] get binary length: %u\n", num, length);
            //hexdump(payload, length);
            memcpy(wsd.bytePacket, payload, length);
            dac_output_voltage(DAC_CHANNEL_1, wsd.sensor.x);
            dac_output_voltage(DAC_CHANNEL_2, wsd.sensor.z);
             // send message to client
            // webSocket.sendBIN(num, payload, length);
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
    Serial.begin(115200);
    Serial1.begin(115200,SERIAL_8N1,RX1_pin,TX1_pin);
    delay(1000);
    
    WiFi.mode(WIFI_AP);
    WiFi.softAP(SSID, PASSWORD);
    server.begin();
 
    dac_output_enable(DAC_CHANNEL_1); //(LOLIN PIN 25)
    dac_output_enable(DAC_CHANNEL_2); //(LOLIN PIN 26)


    //Serial.setDebugOutput(true);
    Serial.setDebugOutput(true);

    Serial.println();
    Serial.println();
    Serial.println();

    for(uint8_t t = 4; t > 0; t--) {
        Serial.printf("[SETUP] BOOT WAIT %d...\n", t);
        Serial.flush();
        delay(1000);
    }

 //   WiFiMulti.addAP("SSID", "passpasspass");

//    while(WiFiMulti.run() != WL_CONNECTED) {
//        delay(100);
//    }

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
  
}

void loop() {
    static unsigned long last = 0;
    webSocket.loop();
    if(abs(millis() - last) > 1000) {
  //      String t = String(millis());
  //      webSocket.broadcastTXT(t); //sendTXT(num,String(millis()));
          last = millis();
          Serial.print("Analog X: ");
          Serial.print(wsd.sensor.x);
          Serial.print(" - Y: ");
          Serial.print(wsd.sensor.y);
          Serial.print(" - Z: ");
          Serial.println(wsd.sensor.z);


    }
    if(wsd.sensor.z < 0 ) wsd.sensor.z = 0;
    if(wsd.sensor.z == 0 ) wsd.sensor.x = 0;
    
    setHoverboardTraction( wsd.sensor.z, wsd.sensor.x ) ;

}

// Binary UART Hoverboard communication

typedef struct MsgToHoverboard_t{
  unsigned char SOM;  // 0x02
  unsigned char len;  // len is len of ALL bytes to follow, including CS
  unsigned char cmd;  // 'W'
  unsigned char code; // code of value to write
  int16_t base_pwm;   // absolute value ranging from -1000 to 1000 .. base_pwm plus/minus steer is the raw PWM value 
  int16_t steer;      // absolute value ranging from -1000 to 1000 .. wether steer is added or substracted depends on the side R/L
  unsigned char CS;   // checksumm
};

typedef union UART_Packet_t{
  MsgToHoverboard_t msgToHover;
  byte UART_Packet[sizeof(MsgToHoverboard_t)];
};


void setHoverboardTraction( int16_t base_pwm, int16_t steer )   
{
  UART_Packet_t ups; 

  ups.msgToHover.SOM = 2 ;  // PROTOCOL_SOM; //Start of Message;  
  ups.msgToHover.len = 7;   // payload + SC only
  ups.msgToHover.cmd  = 'W'; // PROTOCOL_CMD_WRITEVAL;  // Write value
  ups.msgToHover.code = 0x07; // speed data from params array
  ups.msgToHover.base_pwm = base_pwm;
  ups.msgToHover.steer = steer;
  ups.msgToHover.CS = 0;
  
  for (int i = 0; i < ups.msgToHover.len; i++){
    ups.msgToHover.CS -= ups.UART_Packet[i+1];
  }
    
  Serial1.write(ups.UART_Packet,sizeof(UART_Packet_t));
}

// Machine protocol:
// a very simple protocol, starting 02 (SOM), with length and checksum
// examples:
// ack - 02 02 41 BD
// nack - 02 02 4E B0
// test - 02 06 54 54 65 73 74 06
// e.g. for test:
// 02 - SOM
// 06 - length = 6
// 54 - byte0 - 'cmd' 'T'
// 54 - byte1 - payload for text command - 'T'est
// 65 - byte2 - 'e'
// 73 - byte3 - 's'
// 74 - byte4 - 't'
// 06 - checksum = (00 - (06+54+54+65+73+74))&0xff = 06,
// or  can be stated as : (06+54+54+65+73+74+06)&0xff = 0
#endif
