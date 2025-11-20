
void WT5500setup() {
    pinMode(W5500_SS, OUTPUT);
    digitalWrite(W5500_SS, HIGH);
    
    SPI.begin();
    
    Ethernet.init(W5500_SS);
    
    MDL.IP3 = MDL.ID + 50;
    IPAddress LocalIP(MDL.IP0, MDL.IP1, MDL.IP2, MDL.IP3);
    static uint8_t LocalMac[] = { 0x0A, 0x0B, 0x42, 0x0C, 0x0D, MDL.IP3 };
    
    Ethernet.begin(LocalMac, 0);
    Ethernet.setLocalIP(LocalIP);
    IPAddress Mask(255, 255, 255, 0);
    Ethernet.setSubnetMask(Mask);
    IPAddress Gateway(MDL.IP0, MDL.IP1, MDL.IP2, 1);
    Ethernet.setGatewayIP(Gateway);
    
    delay(1500);
    ChipFound = (Ethernet.hardwareStatus() != EthernetNoHardware);
    
    if (ChipFound) {
        if (Ethernet.linkStatus() == LinkON) {
            ETHconnected = true;
            Serial.println("WT5500 Ethernet Connected.");
        } else {
            ETHconnected = false;
            Serial.println("WT5500 Ethernet Not Connected.");
        }
        Serial.print("IP Address: ");
        Serial.println(Ethernet.localIP());
    } else {
        ETHconnected = false;
        Serial.println("No WT5500 ethernet hardware found.");
    }
}
