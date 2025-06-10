
void CheckRelays()
{
      //Load the current pgn relay state - Sections
        for (uint8_t i = 0; i < 8; i++)
        {
            relayState[i] = bitRead(relayLo, i);
        }

        for (uint8_t i = 0; i < 8; i++)
        {
            relayState[i + 8] = bitRead(relayHi, i);
        }

        // Hydraulics
        relayState[16] = isLower;
        relayState[17] = isRaise;

        //Tram
        relayState[18] = bitRead(tramline, 0); //right
        relayState[19] = bitRead(tramline, 1); //left

        //GeoStop
        relayState[20] = (geoStop == 0) ? 0 : 1;

        int IOpin;
        if (PCA9685_found)
        {
            // 2 pins used for each valve, powered on and off, 8 sections
            bool anyActive = false;
            for (int i = 0; i < 12; i++)
            {
              IOpin = i;
              if (pin[i]) {
                    if(activeON == relayState[pin[i] - 1])  // false for monosem
                    {
                        // on  
                        PWMServoDriver.setPWM(IOpin, 0, 4095); //on
                        anyActive = true;
                    }
                    else
                    {
                        // off
                        PWMServoDriver.setPWM(IOpin, 0, 0); //off
                    }
              }
            }
            #ifdef activeON
              if(anyActive && !pin[11])  // to control the master switch with the 12th output
              {
                // on  
                PWMServoDriver.setPWM(11, 0, 4095); //on
              }
              else
              {
                // off
                PWMServoDriver.setPWM(11, 0, 0); //off
              }
            #endif
        }
        if (PCA9685Ext_found)
        {
            // 2 pins used for each valve, powered on and off, 8 sections
            for (int i = 0; i < 8; i++)
            {
              IOpin = i * 2;
              if (pin[8+i]) {
                    if(relayState[pin[8+i] - 1])
                    {
                        // on  
                        PWMServoDriverExt.setPWM(IOpin, 0, 0);
                        PWMServoDriverExt.setPWM(IOpin + 1, 0, 4095); //green
                    }
                    else
                    {
                        // off
                        PWMServoDriverExt.setPWM(IOpin, 0, 4095);  //red
                        PWMServoDriverExt.setPWM(IOpin + 1, 0, 0);
                    }
                }
            }
        }        
}
