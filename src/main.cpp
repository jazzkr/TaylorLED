/**
 * TaylorLED WiFi Code
 * By Krisztian Kurucz
 * Date: 2017-07-31
 */

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "DNSServer.h"
#include "ESP8266WebServer.h"
#include "EEPROM.h"

// ESP WiFi Variables
char* ESP_SSID;
char* ESP_PSK;
const byte DNS_PORT = 53;

// Pin Definitons
const int LED_PIN = LED_BUILTIN;

ESP8266WebServer webServer(80);
DNSServer dnsServer;
IPAddress ap_IP(192, 168, 1, 1);

String responseHTML = ""
  "<!DOCTYPE html><html><head><title>CaptivePortal</title></head><body>"
  "<h1>Hello World!</h1><p>This is a captive portal example. All requests will "
  "be redirected here.</p></body></html>";

void sendCaptivePortal()
{
  webServer.send(200, "text/html", responseHTML);
}

void initWiFiAPMode()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(ap_IP, ap_IP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ESP_SSID);

  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", ap_IP);

  // Make sure regardless of what happens we send the portal
  webServer.on("/", sendCaptivePortal);
  webServer.on("/generate_204", sendCaptivePortal);
  webServer.on("/fwlink", sendCaptivePortal);
  webServer.onNotFound(sendCaptivePortal);
  webServer.begin();
}

void loopWiFiAPMode()
{
  dnsServer.processNextRequest();
  webServer.handleClient();
}

//This function writes [sizeof(ssid)] + ssid + [sizeof(psk)] + psk
void writeSSIDToEEPROM(char ssid[], char psk[], int sizeof_ssid, int sizeof_psk)
{
  int b1 = sizeof_ssid;
  int b2 = sizeof_psk;
  int totalBytes = b1 + b2 + 3;

  Serial.print("sizeof(ssid) = ");
  Serial.println(b1);
  Serial.print("sizeof(psk) = ");
  Serial.println(b2);
  Serial.print("totalBytes = ");
  Serial.println(totalBytes);

  if (totalBytes >= 512) return;

  EEPROM.begin(512);
  //Clear EEPROM before Writing
  Serial.println("Clearing EEPROM now...");
  for (int i = 0; i < 512; i++)
  {
    EEPROM.write(i, 0xFF);
  }
  Serial.println("EEPROM clear!");

  Serial.print("Writing ");
  Serial.print(ssid);
  Serial.print(" / ");
  Serial.print(psk);
  Serial.println(" to EEPROM");

  int eeprom_i = 0;

  EEPROM.write(eeprom_i, b1);
  eeprom_i += 1;

  for (int i = 0; i < b1; i++)
  {
    EEPROM.write(eeprom_i, ssid[i]);
    eeprom_i += 1;
  }

  EEPROM.write(eeprom_i, b2);
  eeprom_i += 1;

  for (int i = 0; i < b2; i++)
  {
    EEPROM.write(eeprom_i, psk[i]);
    eeprom_i += 1;
  }

  EEPROM.end();

  Serial.println("Finished writing to EEPROM!");
  Serial.print("EEPROM End Index: ");
  Serial.println(eeprom_i);

}

// Reads SSID info out from EEPROM
// TODO: properly save the value we read out (pass by reference?)
void readSSIDFromEEPROM(char ESP_SSID[], char ESP_PSK[])
{
  int eeprom_i = 0;
  byte val;

  EEPROM.begin(512);

  val = EEPROM.read(eeprom_i);
  eeprom_i += 1;

  if (val == 0xFF)
  {
    Serial.println("Error: Program has not written to EEPROM!");
    return;
  }

  int ssid_sizeof = val;
  char ssid[ssid_sizeof];

  for (int i = 0; i < ssid_sizeof; i++)
  {
    val = EEPROM.read(eeprom_i);
    ssid[i] = val;
    eeprom_i += 1;
  }

  val = EEPROM.read(eeprom_i);
  eeprom_i += 1;

  int psk_sizeof = val;
  char psk[psk_sizeof];

  for (int i = 0; i < psk_sizeof; i++)
  {
    val = EEPROM.read(eeprom_i);
    psk[i] = val;
    eeprom_i += 1;
  }

  EEPROM.end();

  Serial.println("Successfully read EEPROM!");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("PSK: ");
  Serial.println(psk);

  // Return through setting ESP_SSID & ESP_PSK
  ESP_SSID = ssid;
  ESP_PSK = psk;
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  // Read EEPROM here for stored SSID information
  readSSIDFromEEPROM(ESP_SSID, ESP_PSK);
  delay(2000);
  Serial.print("Read out: ");
  Serial.print(ESP_SSID);
  Serial.print(" / ");
  Serial.println(ESP_PSK);
  // If nothing stored, go into AP mode
  //initWiFiAPMode();

}

void loop()
{
  loopWiFiAPMode();
}
