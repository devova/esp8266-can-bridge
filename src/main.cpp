#include <Arduino.h>

#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>

#include <ESP8266mDNS.h>
#include <WiFiClient.h>

#include <WiFiUdp.h>
#include <ArduinoOTA.h>
// config: ////////////////////////////////////////////////////////////

#define UART_BAUD 460800
#define packTimeout 10 // ms (if nothing more on UART, then send packet)
#define bufferSize 8192
#define commandSize 255

//////////////////////////////////////////////////////////////////////////
const int port = 8088;

WiFiServer server(port);
WiFiClient client;

uint8_t buf1[bufferSize];
uint8_t i1 = 0;

uint8_t buf2[bufferSize];
uint8_t i2 = 0;

String command = "";
String instruction = "";

String new_ssid = "";
String new_pass = "";

// String ssid1 = "";
// String pass1 = "";

// String ssid2 = "";
// String pass2 = "";

// Helper variables
uint8_t h1;

void keepWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFiManager wifiManager;
    wifiManager.autoConnect("CitroenCAN");
    if (MDNS.begin("can")) {
      MDNS.addService("can", "tcp", port);
    } 
    server.begin(); // start TCP server
  }
}

void setup()
{

    delay(500);

    Serial.begin(UART_BAUD);

    keepWiFi();

    ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  MDNS.begin("can");
}

void loop()
{
    keepWiFi();
    ArduinoOTA.handle();

    if (!client.connected())
    {                                // if client not connected
        client = server.available(); // wait for it to connect
        return;
    }

    // here we have a connected client

    if (client.available())
    {
        while (client.available())
        {
            buf1[i1] = (uint8_t)client.read(); // read char from client (RoboRemo app)
            command += (char) buf1[i1];
            if ((char) buf1[i1] == '\n')
            {
                if (command[0] == 'C') // Each command should start from 'C'
                {
                    instruction = command.substring(1, 5);
                    if (instruction == "SSID")
                    {
                        h1 = command.indexOf(':', 5);
                        new_ssid = command.substring(5, h1);
                        new_pass = command.substring(h1 + 1);
                        Serial.println(new_pass);
                        // if (not WiFi_try_new_ssid(new_ssid, new_pass)){
                            
                        // }
                    }
                }
                command = "";
            }
            if (i1 < bufferSize - 1)
                i1++;
        }
        // now send to UART:
        Serial.write(buf1, i1);
        i1 = 0;
    }

    if (Serial.available())
    {

        // read the data until pause:

        while (1)
        {
            if (Serial.available())
            {
                buf2[i2] = (char)Serial.read(); // read char from UART
                if (i2 < bufferSize - 1)
                    i2++;
            }
            else
            {
                //delayMicroseconds(packTimeoutMicros);
                delay(packTimeout);
                if (!Serial.available())
                {
                    break;
                }
            }
        }

        // now send to WiFi:
        client.write((char *)buf2, i2);
        i2 = 0;
    }
    client.write("ping");
}