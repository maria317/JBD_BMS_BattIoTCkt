void writeFileHeader() {
  File logFile = LittleFS.open(logFileName, "a");
  if (!logFile) {
    Serial.println("Failed to open log file");
  } else {
    logFile.println("Seconds,Voltage,Current,SOC,Charge MOS, Discharge MOS, Max Temp, Cell Diff,Cell 1,Cell 2,Cell 3,Cell 4,Cell 5,Cell 6,Cell 7,Cell 8,Cell 9,Cell 10,Cell 11,Cell 12,Cell 13,Cell 14,Cell 15,Cell 16,Cell 17,Cell 18,Cell 19,Cell 20");
    logFile.close();
    Serial.println("Successfully opened File");
  }
}

int zeroslogged = 0;

/**
 * @brief this function logs data in FS file system
 * Gets JBD BMS data (via CAN) by calling get_CAN_data() function
 * Logs data into the file
 */
void logData() {
//  getbmsData93();
//  getbmsData90();
//  getbmsData91();
//  getbmsData92();
//  getbmsData95();

  get_CAN_data();
  if (!LittleFS.exists(logFileName)) {
    writeFileHeader();
  }
  File logFile = LittleFS.open(logFileName, "a");
  if (mybms90.Current != 0) { zeroslogged = 0; }
  if (logFile && (zeroslogged < 12)) {
    if (mybms90.readSuccess90 && mybms92.readSuccess92 && mybms93.readSuccess93) {
      if (mybms90.Current == 0) zeroslogged++;
      logFile.print(millis() / 1000.0);
      logFile.print(",");
      logFile.print(mybms90.Voltage);
      logFile.print(",");
      logFile.print(mybms90.Current);
      logFile.print(",");
      logFile.print(mybms90.SOC);
      logFile.print(",");
      logFile.print(mybms93.charge_mos_state);
      logFile.print(",");
      logFile.print(mybms93.discharge_mos_state);
      logFile.print(",");
      // logFile.print(mybms93.BMS_life);
      // logFile.print(",");
      logFile.print(mybms92.max_temp);
      logFile.print(",");
      logFile.print(mybms91.max_cell_mv - mybms91.min_cell_mv);
      logFile.print(",");
    }
    if (mybms95.readSuccess95) {
      for (int i = 0; i < 20; i++) {
        logFile.print(mybms95.cellVoltages[i]);
        if (i < 19) {
          logFile.print(",");
        }
      }
    }
    if ((mybms90.readSuccess90 && mybms91.readSuccess91 && mybms92.readSuccess92 && mybms93.readSuccess93) || (mybms95.readSuccess95)) { logFile.println(); }
    size_t fileSize = logFile.size();
    //logFile.println("1");
    Serial.print("File size: ");
    Serial.print(fileSize);
    Serial.println(" bytes");
    logFile.close();
    Serial.println("Data logged");
  } else {
    Serial.print("Failed to open log file:");
    Serial.print(logFile);
    Serial.print(",");
    Serial.print(mybms90.readSuccess90);
    Serial.print(",");
    Serial.print(mybms92.readSuccess92);
    Serial.print(",");
    Serial.print(mybms93.readSuccess93);
    Serial.print(",");
    Serial.println(mybms95.readSuccess95);
  }
}

/**
 * @brief This function displays bms values on webpage and allows you to download/delete file via button
 */
void handleRoot(AsyncWebServerRequest* request) {
  String html = "<html><body><table>";
  html += "<h1>ESP8266 File Handling and Data Logging</h1>";
  // Display current values of variables from logData function
  html += "<h2>Current Data Values:</h2>";
  html += "<table border='1'><tr><th>Parameter</th><th>Value</th></tr>";
  html += "<tr><td>Millis</td><td>" + String(millis() / 1000.0) + " s</td></tr>";
  html += "<tr><td>Voltage</td><td>" + String(mybms90.Voltage) + " V</td></tr>";
  html += "<tr><td>Current</td><td>" + String(mybms90.Current) + " A</td></tr>";
  html += "<tr><td>SOC</td><td>" + String(mybms90.SOC) + " %</td></tr>";
  html += "<tr><td>Max cell mV</td><td>" + String(mybms91.max_cell_mv) + " mV</td></tr>";
  html += "<tr><td>Min cell mV</td><td>" + String(mybms91.min_cell_mv) + " mV</td></tr>";
  html += "<tr><td>Cell Diff.</td><td>" + String(mybms91.max_cell_mv - mybms91.min_cell_mv) + " mV</td></tr>";
  html += "<tr><td>Ch. MOS state</td><td>" + String(mybms93.charge_mos_state) + "</td></tr>";
  html += "<tr><td>Disch. MOS state</td><td>" + String(mybms93.discharge_mos_state) + "</td></tr>";
  // html += "<tr><td>Cycles</td><td>" + String(mybms93.BMS_life) + " cycles</td></tr>";
  html += "<tr><td>Max Temperature</td><td>" + String(mybms92.max_temp) + " &#8451;</td></tr>";
  html += "</table>";

  // Display the first few cell voltages
  html += "<h2>Cell Voltages:</h2>";
  html += "<table border='1'><tr><th>Cell Number</th><th>Voltage</th></tr>";
  for (int i = 0; i < 20; i++) {
    html += "<tr><td>Cell " + String(i + 1) + "</td><td>" + String(mybms95.cellVoltages[i]) + " V</td></tr>";
  }
  html += "</table>";
  html += "<p><a href='/download'><button>Download File</button></a><br><br><br></p>";
  html += "<p><a href='/delete'><button>Delete File</button></a></p>";

  html += "</body></html>";
  request->send(200, "text/html", html);
}

void handleDelete(AsyncWebServerRequest* request) {
  //String filename = "/example.txt"; // Change to the file you want to delete
  if (LittleFS.remove(logFileName)) {
    request->send(200, "text/plain", "File deleted successfully");
  } else {
    request->send(404, "text/plain", "File not found or unable to delete");
  }
}
