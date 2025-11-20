#ifndef ETHClass_h
#define ETHClass_h

#include <Arduino.h>

class ETHClass {
public:
    ETHClass();
    bool config(IPAddress local_ip, IPAddress gateway, IPAddress subnet);
    IPAddress localIP();
    
private:
    IPAddress _local_ip;
};

extern ETHClass ETH;

#endif
