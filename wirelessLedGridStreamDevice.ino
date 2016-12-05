/*

When the sketch is uploaded, the ESP8266 will reboot and create its own access point called AutoConnect AP. Connecting to that AP will bring up the 
list of available access points. Enter Username and PW for the access point and ESP module will save the credentials for you. 

When the module is restarted, it will connect to the AP that you chose and should be available on the network. You can get its IP from the Serial monitor 
or use mDNS library (it's uncommented by default, along with all usage of the library) to make the module discoverable.

To change colors on the module (Neopixels on Pin 2), simply go to the root URL of the web server.

*/

#define FASTLED_ESP8266_RAW_PIN_ORDER

#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WebSocketsClient.h>
//#include <WebSocketsServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h> 
#include <FastLED.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Hash.h>

WebSocketsClient webSocket;
//WebSocketsServer webSocket = WebSocketsServer(81);
//ESP8266WebServer server = ESP8266WebServer(80);

ESP8266HTTPUpdateServer httpUpdater;

#define USE_SERIAL Serial
#define DATA_PIN 4
#define CLOCK_PIN 5
#define COLOR_ORDER GRB
#define CHIPSET WS2811
#define BRIGHTNESS 32

const uint8_t kMatrixWidth = 8;
const uint8_t kMatrixHeight = 8;

//get the number of leds from the matrix dimensions
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
//or just set the number of leds directly
//#define NUM_LEDS 144
CRGB leds[NUM_LEDS];

uint8_t gHue = 0;
int mode = 0;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.printf("[WSc] Disconnected!\n");
            connectWS();
            break;
        case WStype_CONNECTED:
            USE_SERIAL.printf("[WSc] Connected to url: %s\n",  payload);
            // send message to server when Connected
				    webSocket.sendTXT("Big Window Device Connected");
            break;
        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] get text: %s\n", payload);
            updateFrame(payload, length);
			      // send message to server
			      // webSocket.sendTXT("message here");
            break;
        case WStype_BIN:
            USE_SERIAL.printf("[WSc] get binary length: %u\n", length);
            //hexdump(payload, length);
            updateFrame(payload, length);
            
            // send data to server
            // webSocket.sendBIN(payload, lenght);
            break;
    }

}

void connectWS() {
    //webSocket.begin("wirelessledgridstream.azurewebsites.net", 80, "/", "device");
    webSocket.begin("192.168.1.152", 3000, "/", "device");
    //webSocket.setAuthorization("user", "Password"); // HTTP Basic Authorization
    webSocket.onEvent(webSocketEvent);

    //if(MDNS.begin("esp8266")) {
    //    USE_SERIAL.println("MDNS responder started");
    //}

    // Add service to MDNS
    //MDNS.addService("http", "tcp", 80);
    //MDNS.addService("ws", "tcp", 81);

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);
  
    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");
  
    // No authentication by default
    // ArduinoOTA.setPassword((const char *)"123");
}

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.setDebugOutput(true);
    USE_SERIAL.println();

    WiFiManager wifiManager;
    
    //TODO: make tie the reset to a GPIO button
    //reset settings - for testing 
    //wifiManager.resetSettings();
    
    wifiManager.autoConnect("AutoConnectAP");

    delay(2000);

    USE_SERIAL.println("Connection established!");
    //FastLED.addLeds<CHIPSET, DATA_PIN, CLOCK_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    
    FastLED.setBrightness( BRIGHTNESS );

    connectWS();
  
    ArduinoOTA.onStart([]() {
      USE_SERIAL.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      USE_SERIAL.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      USE_SERIAL.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      USE_SERIAL.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) USE_SERIAL.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) USE_SERIAL.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) USE_SERIAL.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) USE_SERIAL.println("Receive Failed");
      else if (error == OTA_END_ERROR) USE_SERIAL.println("End Failed");
    });
    ArduinoOTA.begin();
    Serial.println("Ready");
}

void updateFrame(uint8_t * framepayload, size_t length) {

    /*uint32_t frame[] = {0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,
            0x000000,0xFF0000,0x000000,0x00FF00,0x000000,0x0000FF,0x000000,0x000000,
            0x000000,0xFF0000,0x000000,0x00FF00,0x000000,0x0000FF,0x000000,0x000000,
            0x000000,0xFF0000,0x000000,0x00FF00,0x000000,0x0000FF,0x000000,0x000000,
            0x000000,0xFF0000,0x000000,0x00FF00,0x000000,0x0000FF,0x000000,0x000000,
            0x000000,0xFF0000,0x000000,0x00FF00,0x000000,0x0000FF,0x000000,0x000000,
            0x000000,0xFF0000,0x000000,0x00FF00,0x000000,0x0000FF,0x000000,0x000000,
            0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000,0x000000};
    */
    memcpy(&leds, framepayload, length);
    //for(int i=0;i<NUM_LEDS;i++) {
    //    leds[i]=frame[i];
    //}
}

void loop() {
    ArduinoOTA.handle();
    webSocket.loop();
    FastLED.show();
    FastLED.delay(1000/30);
    //webSocket.sendTXT("test");
}
