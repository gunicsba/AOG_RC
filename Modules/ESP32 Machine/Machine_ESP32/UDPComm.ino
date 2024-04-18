


void ReceiveAGIO()
{
    if (ChipFound)
    {
        if (ETHconnected)
        {
            uint16_t len = UDP_AGIO.parsePacket();
            if (len)
            {
                byte Data[MaxReadBuffer];
                UDP_AGIO.read(Data, MaxReadBuffer);
                if ((Data[0] == 128) && (Data[1] == 129) && (Data[2] == 127))  // 127 is source, AGIO
                {
                    switch (Data[3])
                    {
                    case 200:
                        if(Data[7] == 1) {
                          relayLo -= 255;
                          relayHi -= 255;
                          watchdogTimer = 0;
                        }
                        helloFromMachine[5] = relayLo;
                        helloFromMachine[6] = relayHi;
                        if (ChipFound)
                            {
                                if (ETHconnected)
                                {

                                    UDP_Ethernet.beginPacket(Ethernet_DestinationIP, DestinationPortAGIO);
                                    UDP_Ethernet.write(helloFromMachine, sizeof(helloFromMachine));
                                    UDP_Ethernet.endPacket();
                                }
                            }
                    break;
                    case 201:
                        if ((Data[4] == 5) && (Data[5] == 201) && (Data[6] == 201))
                        {
                            MDL.IP0 = Data[7];
                            MDL.IP1 = Data[8];
                            MDL.IP2 = Data[9];

                            SaveData();
                            ESP.restart();
                        }
                        break;
                    case 202:
                    {
                      //make really sure this is the subnet pgn
                      if (Data[4] == 3 && Data[5] == 202 && Data[6] == 202)
                      {
                        uint8_t scanReply[] = { 128, 129, 123, 203, 7, 
                        MDL.IP0, MDL.IP1, MDL.IP2, 123,
                        //src_ip[0], src_ip[1], src_ip[2], 23   };
                        UDP_AGIO.remoteIP()[0], UDP_AGIO.remoteIP()[1], UDP_AGIO.remoteIP()[2], 23   };

                        //checksum
                        int16_t CK_A = 0;
                        for (uint8_t i = 2; i < sizeof(scanReply) - 1; i++)
                        {
                            CK_A = (CK_A + scanReply[i]);
                        }
                        scanReply[sizeof(scanReply)-1] = CK_A;

                        static uint8_t ipDest[] = { 255,255,255,255 };

                        //off to AOG
                        if (ChipFound)
                            {
                                if (ETHconnected)
                                {

                                    UDP_Ethernet.beginPacket(ipDest, DestinationPortAGIO);
                                    UDP_Ethernet.write(scanReply, sizeof(scanReply));
                                    UDP_Ethernet.endPacket();
                                }
                            }

                      }

                        break;
                    }
                    case 236:
                    {
                        for (uint8_t i = 0; i < 24; i++)
                        {
                          pin[i] = Data[i + 5];
                        }

                        //save in EEPROM and restart
                        EEPROM.put(20, pin);
                        EEPROM.commit();
                        break;
                    }
                    case 238:
                    {
                        aogConfig.raiseTime = Data[5];
                        aogConfig.lowerTime = Data[6];
                        aogConfig.enableToolLift = Data[7];

                        //set1 
                        uint8_t sett = Data[8];  //setting0     
                        if (bitRead(sett, 0)) aogConfig.isRelayActiveHigh = 1; else aogConfig.isRelayActiveHigh = 0;

                        aogConfig.user1 = Data[9];
                        aogConfig.user2 = Data[10];
                        aogConfig.user3 = Data[11];
                        aogConfig.user4 = Data[12];

                        //crc

                        //save in EEPROM and restart
                        EEPROM.put(6, aogConfig);
                        EEPROM.commit();
                        //resetFunc();
                      break;
                    }
                    case 239:
                    {
                      uTurn = Data[5];
                      gpsSpeed = (float)Data[6];//actual speed times 4, single uint8_t

                      hydLift = Data[7];
                      tramline = Data[8];  //bit 0 is right bit 1 is left

                      relayLo = Data[11];          // read relay control from AgOpenGPS
                      relayHi = Data[12];

                      if (aogConfig.isRelayActiveHigh)
                      {
                          tramline = 255 - tramline;
                          relayLo = 255 - relayLo;
                          relayHi = 255 - relayHi;
                      }

                      //Bit 13 CRC

                      //reset watchdog
                      watchdogTimer = 0;
                      break;
                    }
                  }
                }
            }
        }
    }
}

void SendAGIO(){

}


