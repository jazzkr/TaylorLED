/**
 * TaylorLED WiFi Code
 * By Krisztian Kurucz
 * Date: 2017-07-31
 */

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include "DNSServer.h"
#include "ESP8266WebServer.h"

// ESP WiFi Constants
const char SSID[] = "TaylorLED_Setup";
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
  WiFi.softAP(SSID);

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
  //WiFiClient client = server.available();
  //if (!client) return;

  //Serial.println("Incoming request from client...");

  //String req = client.readStringUntil('\r');
  //Serial.print("Request: ");
  //Serial.println(req);
  //client.flush();
  dnsServer.processNextRequest();
  webServer.handleClient();
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);

  initWiFiAPMode();

}

void loop()
{
  loopWiFiAPMode();
}
