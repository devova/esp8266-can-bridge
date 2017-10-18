#include <string>
#include <ESP8266WiFi.h>
#include <EEPROM.h>

String ssid1 = "";
String pass1 = "";

String ssid2 = "";
String pass2 = "";

const uint8_t MAX_ATTEMPTS = 50;
uint8_t attempts = 0;

void WiFi_connect(const String &ssid, const String &pass)
{
    if (WiFi.status() == WL_CONNECTED) WiFi.mode(WIFI_OFF);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), pass.c_str());
    while (WiFi.status() != WL_CONNECTED && attempts < MAX_ATTEMPTS)
    {
        delay(500);
        attempts++;
        Serial.print("Connecting to: ");
        Serial.print(ssid);
        Serial.print(". Attempt: ");
        Serial.println(attempts);
    }
}

bool WiFi_try_new_ssid(const String &ssid, const String &pass)
{
    attempts = 0;
    WiFi_connect(ssid, pass);
    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println(WiFi.localIP());
        EEPROM.put(0, ssid.c_str());
        EEPROM.put(50, pass.c_str());
    }
    return WiFi.status() == WL_CONNECTED;
}