
#define CAN0_INT 16                              // Set INT to pin 2 of arduino of pin 16 of ESP8266 for my case
#define CRC_16_POLYNOMIALS 0xa001


MCP_CAN CAN0(15);    //D8

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];

uint32_t voltage = 0, current = 0, remaining_capacity = 0, full_capacity = 0, num_cycles = 0, rsoc = 0, cell[20];//cell1 = 0, cell2 = 0, cell3 = 0, cell4 = 0, cell5 = 0, cell6 = 0, cell7 = 0, cell8 = 0, cell9 = 0, cell10 = 0, cell11 = 0, cell12 = 0, cell13 = 0, cell14 = 0, cell15 = 0, cell16 = 0, cell17 = 0, cell18 = 0, cell19 = 0, cell20 = 0;
float ex_voltage = 0, ex_current = 0, ex_remaining_capacity = 0, ex_full_capacity = 0, ex_cell[20]; //ex_cell1 = 0, ex_cell2 = 0, ex_cell3 = 0, ex_cell4 = 0, ex_cell5 = 0, ex_cell6 = 0, ex_cell7 = 0, ex_cell8 = 0, ex_cell9 = 0, ex_cell10 = 0, ex_cell11 = 0, ex_cell12 = 0, ex_cell13 = 0, ex_cell14 = 0, ex_cell15 = 0, ex_cell16 = 0, ex_cell17 = 0, ex_cell18 = 0, ex_cell19 = 0, ex_cell20 = 0;
char disp [50];
uint16_t FET_Cont_Status;
uint8_t single_over_voltage, single_under_voltage, group_under_voltage,group_over_voltage, chrg_over_temp, chrg_low_temp, dischrg_over_temp, dischrg_low_temp, chrg_over_crnt, dischrg_over_crnt, shrt_crkt, IC_error, soft_locked_MOS;

byte data[8] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07};
byte data2[8];
byte MOS_OFF[4] = {0x01, 0xFE, 0x01, 0xFE};  //, 0x00, 0x00, 0x00, 0x00}; //, 0x00, 0xFF,0x00, 0xFF};
byte MOS_ONN[4] = {0x00, 0xFF, 0x00, 0xFF};  //, 0x00, 0x00, 0x00, 0x00}; //, 0x00, 0xFF,0x00, 0xFF};

uint16_t CRC_Bytes; 
uint32_t cell_max = 0, cell_min = 5000, cell_diff = 0;
float ex_cell_max = 0.0, ex_cell_min = 5.0, ex_cell_diff = 0.0;

char incoming;


void can_initialized(void)
{
  if(CAN0.begin(MCP_ANY, CAN_1000KBPS, MCP_16MHZ) == CAN_OK) Serial.println("MCP2515 Initialized Successfully!");
  else Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);   // Change to normal mode to allow messages to be transmitted
}


void can_send(long unsigned int id)
{

  // send data:  ID = 0x100, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  //CAN0.sendMsgBuf.rtr = 1;
  byte sndStat = CAN0.sendMsgBuf(id, 0, 8, data);
  //CAN0.setMsg(0x100, 0, 0, 0, data);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
  delay(100);   // send data per 100ms

}





void can_rec(void)
{
    if(1)                         // If CAN0_INT pin is low, read receive buffer
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    if((rxId & 0x80000000) == 0x80000000)     // Determine if ID is standard (11 bits) or extended (29 bits)
    {
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
      Serial.print("extended Id by zain\n");
    }
    else
    {
      sprintf(msgString, "Standard ID: 0x%.3lX       DLC: %1d  Data:", rxId, len);
      Serial.print("standard Id by zain\n");
    }
  
    Serial.print(msgString);
  
    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
      Serial.print("remote request frame Id by zain\n");
    } else {
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
        //Serial.print("non remote request frame Id by zain\n");
      }
    }
        
    Serial.println();
  }
}

void MOS_ON(void)
{
   // send data:  ID = 0x12D, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  //CAN0.sendMsgBuf.rtr = 1;
  byte sndStat = CAN0.sendMsgBuf(0x12D, 0, 4, MOS_ONN);
  //CAN0.setMsg(0x100, 0, 0, 0, data);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
  delay(100);   // send data per 100ms
}

void MOS_OF(void)
{
   // send data:  ID = 0x12D, Standard CAN Frame, Data length = 8 bytes, 'data' = array of data bytes to send
  //CAN0.sendMsgBuf.rtr = 1;
  byte sndStat = CAN0.sendMsgBuf(0x12D, 0, 4, MOS_OFF);
  //CAN0.setMsg(0x100, 0, 0, 0, data);
  if(sndStat == CAN_OK){
    Serial.println("Message Sent Successfully!");
  } else {
    Serial.println("Error Sending Message...");
  }
  delay(100);   // send data per 100ms
}

uint16_t Check_CRC16(uint8_t* pchMsg,uint8_t wDataLen) 
{
  uint8_t i, chChar; 
  uint16_t wCRC = 0xFFFF;
  while (wDataLen--) 
  {
    chChar = *pchMsg++;
    wCRC ^= (uint16_t) chChar;
    for (i = 0; i < 8; i++) 
    {
      if (wCRC & 0x0001) 
        wCRC = (wCRC >> 1) ^ CRC_16_POLYNOMIALS; 
      else 
        wCRC >>= 1;
    }
  } 
  return wCRC; 
}



void get_CAN_data(void)
{

  
can_send(0x100);
  can_rec();
  delay(500);
  
  voltage = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_voltage = (float)voltage / 100;
  
  current = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_current = (float)current / 100; 

  remaining_capacity = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
  ex_remaining_capacity = (float)remaining_capacity / 100; 
    
  sprintf(disp, "The voltage of the battery is : %f", ex_voltage);
  Serial.println(disp);

  sprintf(disp, "The current of the battery is : %f", ex_current);
  Serial.println(disp);

  sprintf(disp, "The remaining capacity of the battery is : %f", ex_remaining_capacity);
  Serial.println(disp);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  can_send(0x101);
  can_rec();
  delay(500);
  
  full_capacity = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_full_capacity = (float)full_capacity / 100;
  
  num_cycles = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  
  rsoc = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
    
  sprintf(disp, "The full_capacity of the battery is : %f", ex_full_capacity);
  Serial.println(disp);

  sprintf(disp, "The number of cycles of the battery is : %d", num_cycles);
  Serial.println(disp);

  sprintf(disp, "The remaining SOC of the battery is : %d", rsoc);
  Serial.println(disp);


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  can_send(0x107);
  can_rec();
  delay(500);
  
  cell[0] = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_cell[0] = (float)cell[0] / 1000;
  
  cell[1] = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_cell[1] = (float)cell[1] / 1000; 

  cell[2] = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
  ex_cell[2] = (float)cell[2] / 1000; 
    
  sprintf(disp, "The cell 1 voltage is : %f", ex_cell[0]);
  Serial.println(disp);

  sprintf(disp,  "The cell 2 voltage is : %f", ex_cell[1]);
  Serial.println(disp);

  sprintf(disp,  "The cell 3 voltage is : %f", ex_cell[2]);
  Serial.println(disp);


    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  can_send(0x108);
  can_rec();
  delay(500);
  
  cell[3] = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_cell[3] = (float)cell[3] / 1000;
  
  cell[4] = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_cell[4] = (float)cell[4] / 1000; 

  cell[5] = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
  ex_cell[5] = (float)cell[5] / 1000; 
    
  sprintf(disp, "The cell 4 voltage is : %f", ex_cell[3]);
  Serial.println(disp);

  sprintf(disp,  "The cell 5 voltage is : %f", ex_cell[4]);
  Serial.println(disp);

  sprintf(disp,  "The cell 6 voltage is : %f", ex_cell[5]);
  Serial.println(disp);


  
    //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  can_send(0x109);
  can_rec();
  delay(500);
  
  cell[6] = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_cell[6] = (float)cell[6] / 1000;
  
  cell[7] = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_cell[7] = (float)cell[7] / 1000; 

  cell[8] = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
  ex_cell[8] = (float)cell[8] / 1000; 
    
  sprintf(disp, "The cell 7 voltage is : %f", ex_cell[6]);
  Serial.println(disp);

  sprintf(disp,  "The cell 8 voltage is : %f", ex_cell[7]);
  Serial.println(disp);

  sprintf(disp,  "The cell 9 voltage is : %f", ex_cell[8]);
  Serial.println(disp);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  can_send(0x10A);
  can_rec();
  delay(500);
  
  cell[9] = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_cell[9] = (float)cell[9] / 1000;
  
  cell[10] = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_cell[10] = (float)cell[10] / 1000; 

  cell[11] = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
  ex_cell[11] = (float)cell[11] / 1000; 
    
  sprintf(disp, "The cell 10 voltage is : %f", ex_cell[9]);
  Serial.println(disp);

  sprintf(disp,  "The cell 11 voltage is : %f", ex_cell[10]);
  Serial.println(disp);

  sprintf(disp,  "The cell 12 voltage is : %f", ex_cell[11]);
  Serial.println(disp);


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  can_send(0x10B);
  can_rec();
  delay(500);
  
  cell[12] = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_cell[12] = (float)cell[12] / 1000;
  
  cell[13] = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_cell[13] = (float)cell[13] / 1000; 

  cell[14] = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
  ex_cell[14] = (float)cell[14] / 1000; 
    
  sprintf(disp, "The cell 13 voltage is : %f", ex_cell[12]);
  Serial.println(disp);

  sprintf(disp,  "The cell 14 voltage is : %f", ex_cell[13]);
  Serial.println(disp);

  sprintf(disp,  "The cell 15 voltage is : %f", ex_cell[14]);
  Serial.println(disp);


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  can_send(0x10C);
  can_rec();
  delay(500);
  
  cell[15] = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_cell[15] = (float)cell[15] / 1000;
  
  cell[16] = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_cell[16] = (float)cell[16] / 1000; 

  cell[17] = ((0xFF00 & (rxBuf[4] << 8) ) | (0x00FF & rxBuf[5]));
  ex_cell[17] = (float)cell[17] / 1000; 
    
  sprintf(disp, "The cell 16 voltage is : %f", ex_cell[15]);
  Serial.println(disp);

  sprintf(disp,  "The cell 17 voltage is : %f", ex_cell[16]);
  Serial.println(disp);

  sprintf(disp,  "The cell 18 voltage is : %f", ex_cell[17]);
  Serial.println(disp);

  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


  can_send(0x10D);
  can_rec();
  delay(500);
  
  cell[18] = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));
  ex_cell[18] = (float)cell[18] / 1000;
  
  cell[19] = ((0xFF00 & (rxBuf[2] << 8) ) | (0x00FF & rxBuf[3]));
  ex_cell[19] = (float)cell[19] / 1000; 
   
  sprintf(disp, "The cell 19 voltage is : %f", ex_cell[18]);
  Serial.println(disp);

  sprintf(disp,  "The cell 20 voltage is : %f", ex_cell[19]);
  Serial.println(disp);

  // float cell_max = 0.0, cell_min = 0.0, cell_diff = 0.0;
  cell_max = 0000;
  cell_min = 5000; 
                       
  ex_cell_max = 0.0;
  ex_cell_min = 5.0; 

  for (int i = 0; i < 20 ; i++)
  {
    if (ex_cell[i] > ex_cell_max)
    {
      ex_cell_max = ex_cell[i];
      cell_max = cell[i];
    }
    if (ex_cell[i] < cell_min)
    {
      ex_cell_min = ex_cell[i];
      cell_min = cell[i];
    }
  }

  cell_diff = (float)(cell_max - cell_min);

  sprintf(disp, "The maximum cell voltage is %f : ", cell_max );
  Serial.println(disp);
  sprintf(disp, "The minimum cell voltage is %f : ", cell_min );
  Serial.println(disp);
  sprintf(disp, "The difference of cell voltage is %f : ", cell_diff);
  Serial.println(disp);
  Serial.println(" ");



  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//uint8_t single_over_voltage, single_under_voltage, group_under_voltage, group_over_voltage, chrg_over_temp, chrg_low_temp, dischrg_over_temp, dischrg_low_temp, chrg_over_crnt, dischrg_over_crnt, shrt_crkt, IC_error, soft_locked_MOS;

  can_send(0x103);
  can_rec();
  delay(500);
  
  FET_Cont_Status = ((0xFF00 & (rxBuf[0] << 8) ) | (0x00FF & rxBuf[1]));

  single_over_voltage = (FET_Cont_Status & 0x0001);
  single_under_voltage = (FET_Cont_Status & 0x0002);
  group_under_voltage = (FET_Cont_Status & 0x0004);
  group_over_voltage = (FET_Cont_Status & 0x0008);
  chrg_over_temp = (FET_Cont_Status & 0x0010);
  chrg_low_temp = (FET_Cont_Status & 0x0020);
  dischrg_over_temp = (FET_Cont_Status & 0x0040);
  dischrg_low_temp = (FET_Cont_Status & 0x0080);
  chrg_over_crnt = (FET_Cont_Status & 0x0100);
  dischrg_over_crnt = (FET_Cont_Status & 0x0200);
  shrt_crkt = (FET_Cont_Status & 0x0400);
  IC_error = (FET_Cont_Status & 0x0800);
  soft_locked_MOS = (FET_Cont_Status & 0x1000);
  
  

  Serial.println(" ");
   
  sprintf(disp, "The single over voltage value : %d", single_over_voltage);
  Serial.println(disp);

  sprintf(disp,  "The single under voltage is : %d", single_under_voltage);
  Serial.println(disp);

  sprintf(disp, "The group under voltage is : %d", group_under_voltage);
  Serial.println(disp);

  sprintf(disp,  "The group over voltage is : %d", group_over_voltage);
  Serial.println(disp);

  sprintf(disp, "The charge over temperature is : %d", chrg_over_temp);
  Serial.println(disp);

  sprintf(disp,  "The charge low temperature is : %d", chrg_low_temp);
  Serial.println(disp);

  sprintf(disp, "The dischrg over temperature is : %d", dischrg_over_temp);
  Serial.println(disp);

  sprintf(disp,  "The dischrg low temperature is : %d", dischrg_low_temp);
  Serial.println(disp);

  sprintf(disp, "The charge over current is : %d", chrg_over_crnt);
  Serial.println(disp);

  sprintf(disp,  "The discharge over current is : %d", dischrg_over_crnt);
  Serial.println(disp);

    sprintf(disp, "The short circuit is : %d", shrt_crkt);
  Serial.println(disp);

  sprintf(disp,  "The IC error is : %d", IC_error);
  Serial.println(disp);

  sprintf(disp,  "The software locked MOS is : %d", soft_locked_MOS);
  Serial.println(disp);

   Serial.println(" ");



  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


//  can_send(0x12F);
//  can_rec();
//  delay(500);
  
//   
//  sprintf(disp, "The cell 19 voltage is : %f", ex_cell19);
//  Serial.println(disp);
//
//  sprintf(disp,  "The cell 20 voltage is : %f", ex_cell20);
//  Serial.println(disp);


  //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//  CRC_Bytes = Check_CRC16(MOS,6); 
//  MOS[7] = CRC_Bytes;
//  MOS[6] = CRC_Bytes >> 8;
//  MOS_ON();
//  can_rec();
//  Serial.print("CRC Value : ");
//  Serial.println(CRC_Bytes, HEX);
//  Serial.println(MOS[6], HEX);
//  Serial.println(MOS[7], HEX);
//  Serial.println(" \n \n ");
//  delay(2000);

  ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

if (Serial.available())
{
  incoming = Serial.read();
  if(incoming == 'a')
  {
    MOS_OF();
    can_rec();
    delay(500);
  }
  else if (incoming == 'b')
  {
    MOS_ON();
    can_rec();
    delay(500);
  }
}

  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

   mybms90.Voltage = ex_voltage;
   mybms90.Current = ex_current;
   mybms90.SOC = rsoc;

   mybms91.max_cell_mv = cell_max;
   mybms91.min_cell_mv = cell_min;

   for (int i = 0; i < 20; i++)
   {
    mybms95.cellVoltages[i] = cell[i];
   }

   mybms90.readSuccess90 = 1;
   mybms91.readSuccess91 = 1;
   mybms92.readSuccess92 = 1;
   mybms93.readSuccess93 = 1;
   mybms95.readSuccess95 = 1;
   
}
