#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h> // curl -F "image=@/tmp/arduino_build_435447/ESP8266Template.ino.bin" myLoc.local/firmware
#include <WiFiManager.h>        //https://github.com/tzapu/WiFiManager
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define LED 2

#define LOCATION "16207TempDisplay"
uint32_t ledTimeout;
#define LEDTIMEOUT 500

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
const char *update_path = "/firmware";
WiFiManager wifiManager;

// wemos OLED is 64x48, also its mapped to x=32,y=0 is the origin
// SCL GPIO5
// SDA GPIO4
// no reset control, arggh!
Adafruit_SSD1306 display(0);

void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400, "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>%s Data</title>\
    <style>\
      body { }\
    </style>\
  </head>\
  <body>\
    <pre>\
    Location:    %s<br>\
    Uptime:      %02d:%02d:%02d<br>\
    </pre>\
  </body>\
</html>", LOCATION, LOCATION, hr, min % 60, sec % 60);
  httpServer.send(200, "text/html", temp);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += httpServer.uri();
  message += "\nMethod: ";
  message += (httpServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += httpServer.args();
  message += "\n";

  for (uint8_t i = 0; i < httpServer.args(); i++) {
    message += " " + httpServer.argName(i) + ": " + httpServer.arg(i) + "\n";
  }

  httpServer.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  Serial.println("\nwemosOLED begin");
  pinMode(LED, OUTPUT);

  displaySetup();
  display.setTextSize(1);
  display.setCursor(32,16);
  display.print("Connecting");
  display.display();
  
  // wifiManager.resetSettings();
  wifiManager.autoConnect(LOCATION);

  httpServer.on("/", handleRoot);
  httpServer.on("/reset", []() {
    httpServer.send(200, "text/plain", "reseting config and hardware\n");
    wifiManager.resetSettings();
    ESP.reset();
  });
  httpServer.onNotFound(handleNotFound);


  httpUpdater.setup(&httpServer, update_path);
  httpServer.begin();
  // Serial.print("IP   ");
  // Serial.println(WiFi.localIP().toString());
  Serial.println("HTTP server started");
  Serial.println("HTTP updater started");
  if (MDNS.begin(LOCATION)) {
      Serial.print ("MDNS responder started http://");
      Serial.print(LOCATION);
      Serial.println(".local");
  }

  display.setCursor(32,16);
  display.print(&WiFi.localIP().toString()[8]);
  display.print("       ");
  display.display();
  delay(5000);
}

void loop(void) {
  httpServer.handleClient();
  if (millis() > ledTimeout) {
    digitalWrite(LED,!digitalRead(LED));
    ledTimeout = millis() + LEDTIMEOUT;
  }
  displayLoop();
}

void displaySetup()   {
  // initialize with the I2C addr 0x3C (for the 64x48)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  display.setTextColor(WHITE,0);
  display.setTextSize(1);
  display.setCursor(32,8);
  display.print(LOCATION);
  display.display();
  delay(20); 
  display.setTextSize(2);
}

unsigned char i;

void displayLoop() {
  display.setTextSize(2);
  display.setCursor(32,16);

  display.print(i);
  display.print("       ");
  display.display();
  i++;
  delay(10);
}

