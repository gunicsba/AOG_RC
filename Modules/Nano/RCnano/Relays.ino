byte Rlys;
uint8_t NewLo;
uint8_t NewHi;

void CheckRelays()
{
    NewLo = 0;
    NewHi = 0;

    if (WifiSwitchesEnabled)
    {
        // wifi relay control
        // controls by relay # not section #
        if (millis() - WifiSwitchesTimer > 30000)   // 30 second timer
        {
            // wifi switches have timed out
            WifiSwitchesEnabled = false;
        }
        else
        {
            if (WifiSwitches[2])
            {
                // wifi master on
                NewLo = WifiSwitches[3];
                NewHi = WifiSwitches[4];
            }
            else
            {
                // wifi master off
                WifiSwitchesEnabled = false;
            }
        }
    }
    else if (Sensor[0].FlowEnabled || Sensor[1].FlowEnabled)
    {
        {
            // normal relay control
            NewLo = RelayLo;
            NewHi = RelayHi;
        }
    }

    // power relays, always on
    NewLo |= PowerRelayLo;
    NewHi |= PowerRelayHi;

    if (MDL.UseMCP23017 )
    {
      if(PCAFound)
      {
          for(int i = 0 ; i < 8; i++)  
          {
            if (bitRead(NewLo, i)) setPosition(i, 1); else setPosition(i, -1);
            if (bitRead(NewHi, i)) setPosition(i+8, 1); else setPosition(i+8, -1);
          }
      }
    }
    else
    {
        // use Nano pins
        for (int j = 0; j < 2; j++)
        {
            if (j < 1) Rlys = NewLo; else Rlys = NewHi;
            for (int i = 0; i < 8; i++)
            {
                if (MDL.Relays[i] > 1) // check if relay is enabled
                {
                    if (bitRead(Rlys, i)) digitalWrite(MDL.Relays[i + j * 8], MDL.RelayOnSignal); else digitalWrite(MDL.Relays[i + j * 8], !MDL.RelayOnSignal);
                }
            }
        }
    }
}
