Overview

The program establishes a WiFi connection and starts a web server. It communicates with the BMS over CAN bus to retrieve data like voltage, current, remaining capacity, cell voltages, and various BMS status flags. The retrieved data is likely stored in variables and potentially logged to a CSV file on the LittleFS. The web server provides functionalities to access and download the logged data.
The code communicates with the DALY BMS via CAN. It works in a way that first the respective data IDs are sent to DALY in order to request data from it. If the data IDs match, DALY responds with the respective data which is than decoded. 
The data is then stored in a file and also displayed on a web server. The file can be downloaded and deleted from the server which provides this interface. 
 
IDs:
90: voltage, current, SoC
91: max cell voltage, min cell voltage, no. of cell with max voltage and no. of cell with min voltage
92: maximum temperature, minimum temperature, cell no. with max temp and cell no. with min temp
93: Charge MOS State, Discharge MOS State, BMS Life and Remaining Capacity
94: Battery string, Charger status, load status and DIO state
95: Individual cell voltages
96: Cell temperature

Function of the individual files:
-) BMSDataloggerFile.ino (main program):

This file contains the main program logic for the BMS data logger.
It includes libraries for WiFi, web server, and LittleFS (a file system for microcontrollers).
It defines functions for setting up WiFi, data logging, handling web requests (for downloading and deleting logs), and debugging mode.
In debugging mode, users can change debug level, remove log file, and enable/disable the web server.


-) batterystructs.h:

This file defines structures for storing various data points received from the BMS, such as voltage, current, SoC (State of Charge), cell voltages, temperatures, etc.
Each data point has a corresponding struct member variable with an appropriate data type.


-) debugging.ino:

This file implements debugging functionalities.
It allows users to enter a settings mode by pressing 's' key on the serial monitor.
In settings mode, users can:
Increase/decrease debug level
Exit settings mode
Remove log file
Read log file contents
Enable/disable the web server


Can_JBD.ino:
Code initializes CAN bus for communication with BMS.
Code defines functions to send/receive CAN messages and control a BMS MOSFET.
Core function retrieves data from BMS (voltage, current, etc.) and stores it.
Serial communication allows turning MOSFET on/off and reading CAN bus state.