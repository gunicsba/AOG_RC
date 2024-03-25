#define ETH_MISO_PIN                    11
#define ETH_MOSI_PIN                    12
#define ETH_SCLK_PIN                    10
#define ETH_CS_PIN                      9  ///???
#define ETH_INT_PIN                     13
#define ETH_RST_PIN                     14
#define ETH_ADDR                        1  ////???

void WT5500setup() {
    WiFi.onEvent(WiFiEvent);

    if (!ETH.beginSPI(ETH_MISO_PIN, ETH_MOSI_PIN, ETH_SCLK_PIN, ETH_CS_PIN, ETH_RST_PIN, ETH_INT_PIN)) {
        Serial.println("ETH start Failed!");
    } else {
      ChipFound = true;
    }
}


void WiFiEvent(WiFiEvent_t event)
{
    switch (event) {
    case ARDUINO_EVENT_ETH_START:
        Serial.println("ETH Started");
        //set eth hostname here
        ETH.setHostname("esp32-ethernet");
        break;
    case ARDUINO_EVENT_ETH_CONNECTED:
        Serial.println("ETH Connected");
        break;
    case ARDUINO_EVENT_ETH_GOT_IP:
        Serial.print("ETH MAC: ");
        Serial.print(ETH.macAddress());
        Serial.print(", IPv4: ");
        Serial.print(ETH.localIP());
        if (ETH.fullDuplex()) {
            Serial.print(", FULL_DUPLEX");
        }
        Serial.print(", ");
        Serial.print(ETH.linkSpeed());
        Serial.println("Mbps");
        ETHconnected = true;
        break;
    case ARDUINO_EVENT_ETH_DISCONNECTED:
        Serial.println("ETH Disconnected");
        ETHconnected = false;
        break;
    case ARDUINO_EVENT_ETH_STOP:
        Serial.println("ETH Stopped");
        ETHconnected = false;
        break;
//WiFi
/*
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
		    Serial.println("WiFi connected.");
		    Serial.print("IP address: ");
		    Serial.println(WiFi.localIP());
		    Serial.println("");
		    WifiComm.begin(WiFi.localIP(), ListeningPort);
		    ESPconnected = true;

		    WifiDestinationIP = WiFi.localIP();
		    WifiDestinationIP[3] = 255;
		    break;

  	case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
	    	Serial.println("WiFi lost connection");
		    Serial.println("");
		    ESPconnected = false;
		    WiFi.disconnect();		// prevent auto reconnect
		    break;
*/
    default:
        Serial.print("ETH event:");
        Serial.println(event);
        break;
    }
}

void testClient(const char *host, uint16_t port)
{
    Serial.print("\nconnecting to ");
    Serial.println(host);

    WiFiClient client;
    if (!client.connect(host, port)) {
        Serial.println("connection failed");
        return;
    }
    client.printf("GET / HTTP/1.1\r\nHost: %s\r\n\r\n", host);
    while (client.connected() && !client.available())
        ;
    while (client.available()) {
        Serial.write(client.read());
    }

    Serial.println("closing connection\n");
    client.stop();
}