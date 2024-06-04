
/**
 * @brief In this file, they are resetting the configurations and reading the file
 */
void debugging() {
  char t = 'z';
  while (1) {
    if (t != '\n') {
      Serial.println("SETTINGS");
      Serial.print("a: Change Debug Level:");
      Serial.println(DEBUGLVL);
      Serial.println("b: Exit");
      Serial.println("c: Remove Log File");
      Serial.println("e: Read Log File");
      Serial.print("f: Change Webpage Enable:");
      Serial.println(webpageEnable);
    }
    while (!Serial.available());
    t = Serial.read();
    if (t == 'a') {
      DEBUGLVL += 1;
      if (DEBUGLVL > MAXDEBUGLVL) DEBUGLVL = 0;
      EEPROM.write(DEBUG_ADDRESS, DEBUGLVL);
    }
    else if (t == 'b') {
      EEPROM.commit();
      break;
    }
    else if (t == 'c') {
      if(LittleFS.remove(logFileName)){
      Serial.println("File Removed");}
      else {Serial.println("File Does not exist");}
      //resetConfigs();
      //Serial.println("EEPROM erased");
      //loadConfigs();
    }
    else if (t == 'e') {
      readLogFile();
    }
    else if (t == 'f') {
      webpageEnable = !webpageEnable;
      EEPROM.write(WEBPAGE_ENABLE, webpageEnable);
    }
  }
  EEPROM.commit();
}


void loadConfigs() {
  DEBUGLVL = EEPROM.read(DEBUG_ADDRESS);
  canEnable = (EEPROM.read(CAN_ENABLE) > 0);
  webpageEnable = (EEPROM.read(WEBPAGE_ENABLE) > 0);
}

void setupEEPROM()
{
  EEPROM.begin(512);
  delay(1000);
}


void resetConfigs() {
  EEPROM.write(DEBUG_ADDRESS, 1);
  EEPROM.write(CAN_ENABLE, 1);
  EEPROM.write(WEBPAGE_ENABLE, 1);
  EEPROM.commit();
  delay(1000);
}

void readLogFile() {
  // Open the log file in read mode
  File logFile = LittleFS.open(logFileName, "r");
  if (logFile) {
    // Print the contents of the log file
    while (logFile.available()) {
      Serial.write(logFile.read());
    }
    logFile.close();
  } else {
    Serial.println("Failed to open log file");
  }

  // Close LittleFS
  LittleFS.end();
}
