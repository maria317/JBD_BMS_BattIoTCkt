//LittleFS lets you access the flash memory
//It allows to read, write, close, and delete files.
//Are they creating a file and sending it to an http server

//ESP8266 has been set to access point mode.
//Maybe we can type ip of esp8266 to view the web page ---> confirm this

#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <mcp_can.h>
#include <SPI.h>
#include "LittleFS.h"  // For LittleFS
// File to log data
const char* logFileName = "/data_log.csv";
#include "batterystructs.h"

//#include <WiFiClient.h>
#include <EEPROM.h>
#include <mcp_can.h>
#include <SoftwareSerial.h>
//#include <ESP8266WebServer.h>
#include <SPI.h>
#include <ArduinoJson.h>

/**
 * @brief Struct variables have been created for different DALY BMS IDs
 */
bmsData90 mybms90;
bmsData91 mybms91;
bmsData92 mybms92;
bmsData93 mybms93;
bmsData95 mybms95;  //Defined Globally to avoid memory leakage
bmsData96 mybms96;  //Defined Globally to avoid memory leakage

/**
 * @brief Sets the SSID and password
 */
const char* ssid = "Zain";
const char* password = "12345678";

/**
 * @brief AsyncWebServer object created to set up the ESP32 web server
 */
AsyncWebServer server(80);

/**
 * @brief SoftwareSerial object created for serail(uart) communication
 */
SoftwareSerial Serialx(D1, D2);

bool APmode=0;      //1 for turning ON AP mode. 0 for Station Mode


uint8_t DEBUGLVL = 1;
bool canEnable = 1, webpageEnable;

void can_initialized(void);
void can_send(long unsigned int);
void can_rec(void);
void MOS_ON(void);
void MOS_OF(void);
uint16_t Check_CRC16(uint8_t* ,uint8_t);
void get_CAN_data(void);
void debugging();


/**
 * @brief Arduino setup to define initial properties
 * CAN initialized
 * LittleFS is initialized which would allow to create a file, write data to it and send it to server
 * ESP8266 has been set to the access point mode
 */
void setup() {
  Serial.begin(115200);
  delay(5000);
  //setupUart();
  can_initialized();
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }
  Serial.println("LittleFS Mounted");
  if (APmode) {
    WiFi.mode(WIFI_AP);                // Set ESP8266 to AP mode
    WiFi.softAP(getssid(), password);  // Start the Access Point
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  } else {
    WiFi.begin(ssid, password);
  }
  if (!LittleFS.exists(logFileName)) {
    writeFileHeader();
  }


  server.on("/", HTTP_GET, handleRoot);
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest* request) {
    if (LittleFS.exists(logFileName)) {
      File file = LittleFS.open(logFileName, "r");
      //request->send(LittleFS, logFileName, "application/octet-stream");
      request->send(file, "application/" + String(logFileName), String(logFileName), true);
    } else {
      request->send(404, "text/plain", "File not found");
    }
  });
  server.on("/delete", HTTP_GET, handleDelete);
  // Start the server
  server.begin();
  Serial.println("HTTP server started");
  Serial.print("DEBUG Level = ");
  Serial.println(DEBUGLVL);

  if (DEBUGLVL > 0) {
    Serial.println(WiFi.macAddress());
  }
  if ((Serial.available()) && (Serial.read() == 's')) {  //Enter Settings mode
    debugging();
  }
  //makeGarbageDataCsv();
}

unsigned long previousMillis = 0;

/**
 * @brief loop function logging data in FS file system after every 5 seconds
 * Data is getting logged every 5 seconds
 */
void loop() {
  unsigned long currentMillis = millis();  // Get the current time
  if (currentMillis - previousMillis >= 5000) {
    previousMillis = currentMillis;
    logData();
  }
}

/**
 * @brief Writing 100,000 bytes of data to the file
 * This function is not being used
 */
void makeGarbageDataCsv() {
  File dataFile = LittleFS.open(logFileName, "w");
  if (!dataFile) {
    Serial.println("Failed to create file");
    return;
  }
  Serial.println("File created");

  // Write data to the file (example data)
  // Note: Writing a large file might require buffering and chunking the data
  for (int i = 0; i < 250000; ++i) { //100,000 bytes of data getting written
    // Example data format: CSV format with index and value
    dataFile.print(i);
    dataFile.print(",");
    dataFile.println(random(1000));  // Random value, replace with your data
  }

  // Close the file
  dataFile.close();
}

String getssid() {  //last 4 bytes of mac based ssid
  uint8_t mac[6];
  WiFi.macAddress(mac);
  char baseMacChr[18] = { 0 };                      //%02X:%02X:%02X:%02X:
  sprintf(baseMacChr, "%02X%02X", mac[4], mac[5]);  //mac[0], mac[1], mac[2], mac[3],
  String myssid = "BatLogger_" + String(baseMacChr);
  return myssid;
}
