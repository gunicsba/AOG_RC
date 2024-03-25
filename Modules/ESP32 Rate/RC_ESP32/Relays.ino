
bool RelayStatus[16];
uint8_t Relays8[] = { 7,5,3,1,8,10,12,14 }; // 8 relay module and a PCA9535PW
uint8_t Relays16[] = { 15,14,13,12,11,10,9,8,0,1,2,3,4,5,6,7 }; // 16 relay module and a PCA9535PW

void CheckRelays()
{
    uint8_t Rlys;
    bool BitState;
    uint8_t IOpin;

    uint8_t NewLo = 0;
    uint8_t NewHi = 0;

    if (WifiMasterOn)
    {
        // wifi relay control
        // controls by relay # not section #
        if (millis() - WifiSwitchesTimer > 30000)   // 30 second timer
        {
            // wifi switches have timed out
            WifiMasterOn = false;
        }
        else
        {
            // set relays
            for (int i = 0; i < 8; i++)
            {
                if (Button[i]) bitSet(NewLo, i);
                if (Button[i + 8]) bitSet(NewHi, i);
            }
        }
    }
    else if (Sensor[0].FlowEnabled || Sensor[1].FlowEnabled)
    {
        // normal relay control
        NewLo |= RelayLo;
        NewHi |= RelayHi;
    }
    else
    {
        // inverted relays, 1 is off
        NewLo |= InvertedLo;
        NewHi |= InvertedHi;
    }

    // power relays, always on
    NewLo |= PowerRelayLo;
    NewHi |= PowerRelayHi;


}
