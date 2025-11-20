#include "ETHClass.h"

ETHClass ETH;

ETHClass::ETHClass() {
}

bool ETHClass::config(IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
    _local_ip = local_ip;
    // Configuration is handled by WT5500setup()
    return true;
}

IPAddress ETHClass::localIP() {
    return _local_ip;
}
