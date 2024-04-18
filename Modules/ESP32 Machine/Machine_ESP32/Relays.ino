
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

        if (pin[0]) digitalWrite(4, relayState[pin[0] - 1]);
        if (pin[1]) digitalWrite(5, relayState[pin[1] - 1]);
        if (pin[2]) digitalWrite(6, relayState[pin[2] - 1]);
        if (pin[3]) digitalWrite(7, relayState[pin[3] - 1]);

        if (pin[4]) digitalWrite(8, relayState[pin[4] - 1]);
        if (pin[5]) digitalWrite(9, relayState[pin[5] - 1]);  
  
        if (PCA9685_found)
        {
          int IOpin;
            // 2 pins used for each valve, powered on and off, 8 sections
            for (int i = 0; i < 8; i++)
            {
              IOpin = i * 2;
              if (pin[i]) {
                    if(relayState[pin[i] - 1])
                    {
                        // on  
                        PWMServoDriver.setPWM(IOpin, 0, 4095);
                        PWMServoDriver.setPWM(IOpin + 1, 0, 0);
                    }
                    else
                    {
                        // off
                        PWMServoDriver.setPWM(IOpin, 0, 0);
                        PWMServoDriver.setPWM(IOpin + 1, 0, 4095);
                    }
              }
            }
        }
}
