
// valid pins for each processor
uint8_t ValidPins0[] = { 0,2,4,13,14,15,16,17,21,22,25,26,27,32,33 };	// SPI pins 5,18,19,23 excluded for ethernet module


void DoSetup()
{
	uint8_t ErrorCount;
	int ADS[] = { 0x48,0x49,0x4A,0x4B };	// ADS1115 addresses

	Sensor[0].FlowEnabled = false;
	Sensor[1].FlowEnabled = false;

	Serial.begin(38400);
	delay(3000);
	Serial.println("");
	Serial.println("");
	Serial.println("");
	Serial.println(InoDescription);
	Serial.println("");

	// eeprom
	EEPROM.begin(EEPROM_SIZE);
	LoadData();

	if (MDL.SensorCount > MaxProductCount) MDL.SensorCount = MaxProductCount;

	Serial.println("");
	Serial.print("Module ID: ");
	Serial.println(MDL.ID);
	Serial.print("Module Version: ");
	Serial.println(InoID);
	Serial.println("");

	// I2C
  Wire.begin(8,18,400000);			// I2C on pins SDA 8, SCL 18, data rate to 400kHz
  scanI2CDevices();
  
	// ADS1115
	if (MDL.AdsAddress == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			ADS1115_Address = ADS[i];
			Serial.print("Starting ADS1115 at address ");
			Serial.println(ADS1115_Address);
			ErrorCount = 0;
			while (!ADSfound)
			{
				Wire.beginTransmission(ADS1115_Address);
				Wire.write(0b00000000);	//Point to Conversion register
				Wire.endTransmission();
				Wire.requestFrom(ADS1115_Address, 2);
				ADSfound = Wire.available();
				Serial.print(".");
				delay(500);
				if (ErrorCount++ > 5) break;
			}
			Serial.println("");
			if (ADSfound)
			{
				Serial.print("ADS1115 connected at address ");
				Serial.println(ADS1115_Address);
				Serial.println("");
				break;
			}
			else
			{
				Serial.print("ADS1115 not found.");
				Serial.println("");
			}
		}
	}
	else
	{
		ADS1115_Address = MDL.AdsAddress;
		Serial.print("Starting ADS1115 at address ");
		Serial.println(ADS1115_Address);
		ErrorCount = 0;
		while (!ADSfound)
		{
			Wire.beginTransmission(ADS1115_Address);
			Wire.write(0b00000000);	//Point to Conversion register
			Wire.endTransmission();
			Wire.requestFrom(ADS1115_Address, 2);
			ADSfound = Wire.available();
			Serial.print(".");
			delay(500);
			if (ErrorCount++ > 5) break;
		}
		Serial.println("");
		if (ADSfound)
		{
			Serial.print("ADS1115 connected at address ");
			Serial.println(ADS1115_Address);
			Serial.println("");
		}
		else
		{
			Serial.print("ADS1115 not found.");
			Serial.println("");
		}
	}

	if (!ADSfound)
	{
		Serial.println("ADS1115 disabled.");
		Serial.println("");
	}

	// ethernet 
	Serial.println("Starting Ethernet ...");
	MDL.IP3 = MDL.ID + 50;
	IPAddress LocalIP(MDL.IP0, MDL.IP1, MDL.IP2, MDL.IP3);
	static uint8_t LocalMac[] = { 0x0A,0x0B,0x42,0x0C,0x0D,MDL.IP3 };

	IPAddress Mask(255, 255, 255, 0);
	IPAddress Gateway(MDL.IP0, MDL.IP1, MDL.IP2, 1);
	Ethernet_DestinationIP = IPAddress(MDL.IP0, MDL.IP1, MDL.IP2, 255);	// update from saved data

  WT5500setup();

  // write confir for static IP, gateway,subnet,dns1,dns2
    if (ETH.config(LocalIP, Gateway, Mask) == false) {
      Serial.println("WT5500 Configuration failed.");
    } else {
      Serial.println("WT5500 Configuration success.");
    }

    int timeout = 10;
    while (!ETHconnected && --timeout >= 0) {
      Serial.print("Linkup:");
      Serial.print(ETH.linkUp());
      
      Serial.print("Linkspeed:");
      Serial.print(ETH.linkSpeed());

      Serial.print("LocalIP:");
      Serial.print(ETH.localIP());
      Serial.println("  Wait for network connect ..."); 
      delay(500);
    }
  Serial.println("UDP begin ");
	// UDP
	UDP_Ethernet.begin(ListeningPort);
  
  // AGIO
	UDP_AGIO.begin(ListeningPortAGIO);

  Serial.println("Sensors setup ");

	// sensors
	for (int i = 0; i < MDL.SensorCount; i++)
	{
    Serial.print("Flow");
    Serial.print(i);
    Serial.print(" pin: ");
    Serial.print(Sensor[i].FlowPin);
    Serial.print(" IN1: ");
    Serial.print(Sensor[i].IN1);
    Serial.print(" IN2: ");
    Serial.println(Sensor[i].IN2);    
		pinMode(Sensor[i].FlowPin, INPUT_PULLUP);
		pinMode(Sensor[i].IN1, OUTPUT);
		pinMode(Sensor[i].IN2, OUTPUT);

		switch (i)
		{
		case 0:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR0, CHANGE);
			break;
		case 1:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR1, CHANGE);
			break;
		}

		// pwm
		// DRV8870 IN1
		ledcSetup(i * 2, 500, 8);
		ledcAttachPin(Sensor[i].IN1, i * 2);
		
		// DRV8870 IN2
		ledcSetup(i * 2 + 1, 500, 8);
		ledcAttachPin(Sensor[i].IN2, i * 2 + 1);
	}

  pinMode(13, OUTPUT); //Cytron
  digitalWrite(13,HIGH);
	
  Serial.println("Relays setup ");

	// Relays
	switch (MDL.RelayControl)
	{
	case 1:
		// Relay GPIO Pins
		Serial.println("");
		Serial.println("Using GPIO pins for relays.");
		for (int i = 0; i < 16; i++)
		{
			if (MDL.RelayPins[i] < NC)
			{
				pinMode(MDL.RelayPins[i], OUTPUT);
			}
		}
		break;

	case 2:
	case 3:
		// PCA9555 I/O expander on default address 0x20
		Serial.println("");
		Serial.println("Starting PCA9555 I/O Expander ...");
		ErrorCount = 0;
		while (!PCA9555PW_found)
		{
			Serial.print(".");
			Wire.beginTransmission(0x20);
			PCA9555PW_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(500);
			if (ErrorCount > 5) break;
		}

		Serial.println("");
		if (PCA9555PW_found)
		{
			Serial.println("PCA9555 expander found.");

			PCA.attach(Wire);
			PCA.polarity(PCA95x5::Polarity::ORIGINAL_ALL);
			PCA.direction(PCA95x5::Direction::OUT_ALL);
			PCA.write(PCA95x5::Level::H_ALL);
		}
		else
		{
			Serial.println("PCA9555 expander not found.");
		}
		break;

	case 4:
		// MCP23017 I/O expander on default address 0x20
		Serial.println("");
		Serial.println("Starting MCP23017 ...");
		ErrorCount = 0;
		while (!MCP23017_found)
		{
			Serial.print(".");
			Wire.beginTransmission(0x20);
			MCP23017_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(500);
			if (ErrorCount > 5) break;
		}

		Serial.println("");
		if (MCP23017_found)
		{
			Serial.println("MCP23017 found.");
			MCP.begin_I2C();

			for (int i = 0; i < 16; i++)
			{
				MCP.pinMode(MDL.RelayPins[i], OUTPUT);
			}
		}
		else
		{
			Serial.println("MCP23017 not found.");
		}
		break;

	case 5:
	case 6:
		// PCA9685
		Serial.println("");
		Serial.println("Starting PCA9685 I/O Expander ...");
		ErrorCount = 0;
		while (!PCA9685_found)
		{
			Serial.print(".");
			Wire.beginTransmission(PCAaddress);
			PCA9685_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(500);
			if (ErrorCount > 5)break;
		}

		Serial.println("");
		if (PCA9685_found)
		{
			Serial.println("PCA9685 expander found.");
			PWMServoDriver.begin();
			PWMServoDriver.setPWMFreq(200);

//			pinMode(OutputEnablePin, OUTPUT);
//			digitalWrite(OutputEnablePin, LOW);	//enable
		}
		else
		{
			Serial.println("PCA9685 expander not found.");
		}

		ErrorCount = 0;
		while (!PCA9685Ext_found)
		{
			Serial.print(".");
			Wire.beginTransmission(PCAExtaddress);
			PCA9685Ext_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(500);
			if (ErrorCount > 5)break;
		}

		Serial.println("");
		if (PCA9685Ext_found)
		{
			Serial.println("PCA9685Ext expander found.");
			PWMServoDriverExt.begin();
			PWMServoDriverExt.setPWMFreq(200);

//			pinMode(OutputEnablePin, OUTPUT);
//			digitalWrite(OutputEnablePin, LOW);	//enable
		}
		else
		{
			Serial.println("PCA9685Ext expander not found.");
		}
		break;

	case 7:
		// PCF8574
		Serial.println("");
		Serial.println("Starting PCF8574 I/O Expander ...");
		ErrorCount = 0;
		while (!PCF_found)
		{
			Serial.print(".");
			Wire.beginTransmission(PCFaddress);
			PCF_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(500);
			if (ErrorCount > 5) break;
		}

		Serial.println("");
		if (PCF_found)
		{
			Serial.println("PCF8574 expander found.");
			PCF.begin();
		}
		else
		{
			Serial.println("PCF8574 expander not found.");
		}
		break;
	}
	
	// Wifi
	WiFi.mode(WIFI_MODE_APSTA);
	WiFi.disconnect(true);

	// Access Point
	IPAddress AP_LocalIP = IPAddress(192, 168, MDL.ID + 200, 1);
	Wifi_DestinationIP = IPAddress(192, 168, MDL.ID + 200, 255);
	
	String AP = MDL.APname;
	AP += "  (";
	AP += WiFi.macAddress();
	AP += ")";

	WiFi.softAP(AP, MDL.APpassword, 1, 0, 2);
	delay(500);
	WiFi.softAPConfig(AP_LocalIP, AP_LocalIP, AP_Subnet);
	UDP_Wifi.begin(ListeningPort);

	Serial.println("");
	Serial.print("Access Point name: ");
	Serial.println(AP);
	Serial.print("Access Point IP: ");
	Serial.println(AP_LocalIP);

	// web server
	Serial.println();
	Serial.println("Starting Web Server");
	server.on("/", HandleRoot);
	server.on("/page1", HandlePage1);
	server.on("/page2", HandlePage2);
	server.on("/ButtonPressed", ButtonPressed);
  server.on("/info", HandleInfo);
  server.on("/Cytron", Cytron);
	server.onNotFound(HandleRoot);

	// OTA
	server.on("/myurl", HTTP_GET, []() {
		server.sendHeader("Connection", "close");
		server.send(200, "text/plain", "Hello there!");
	});

	server.begin();

	/* INITIALIZE ESP2SOTA LIBRARY */
	ESP2SOTA.begin(&server);
	
	Serial.println("OTA started.");

	// wifi client mode
	if (MDL.WifiMode == 1)
	{
		// connect to network
		delay(1000);
		WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
		WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
		WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
		WiFi.begin(MDL.SSID, MDL.Password);
		Serial.println();
		Serial.println("Connecting to wifi network ...");
	}

	delay(1500);
	Serial.println("");
	Serial.println("Finished setup.");
	Serial.println("");
}

void LoadData()
{
	bool IsValid = false;
	int16_t StoredID;
	int8_t StoredType;
	EEPROM.get(0, StoredID);
	EEPROM.get(4, StoredType);
	if (StoredID == InoID && StoredType == InoType)
	{
		// load stored data
		Serial.println("Loading stored settings.");
		EEPROM.get(110, MDL);

		for (int i = 0; i < MaxProductCount; i++)
		{
			EEPROM.get(300 + i * 80, Sensor[i]);
		}
		IsValid = ValidData();
		if (!IsValid)
		{
			Serial.println("Stored settings not valid.");
		}
	}

	if (!IsValid)
	{
		LoadDefaults();
		SaveData();
	}
}

void SaveData()
{
	// update stored data
	Serial.println("Updating stored settings.");
	EEPROM.put(0, InoID);
	EEPROM.put(4, InoType);
	EEPROM.put(110, MDL);

	for (int i = 0; i < MaxProductCount; i++)
	{
		EEPROM.put(300 + i * 80, Sensor[i]);
	}
	EEPROM.commit();
}

void LoadDefaults()
{
	Serial.println("Loading default settings.");

	// default flow pins
	Sensor[0].FlowPin = 21;
/*
	Sensor[0].IN1 = 4;
	Sensor[0].IN2 = 5;

	Sensor[1].FlowPin = 47;
	Sensor[1].IN1 = 15;
	Sensor[1].IN2 = 7;
*/
	Sensor[1].IN1 = 4;
	Sensor[1].IN2 = 5;

	Sensor[1].FlowPin = 47;
	Sensor[0].IN1 = 15;
	Sensor[0].IN2 = 7;

	// default pid
	Sensor[0].KP = 5;
	Sensor[0].KI = 0;
	Sensor[0].KD = 0;
	Sensor[0].MinPWM = 5;
	Sensor[0].MaxPWM = 50;
	Sensor[0].Debounce = 3;

	Sensor[1].KP = 5;
	Sensor[1].KI = 0;
	Sensor[1].KD = 0;
	Sensor[1].MinPWM = 5;
	Sensor[1].MaxPWM = 50;
	Sensor[1].Debounce = 3;

	// relay pins
	for (int i = 0; i < 16; i++)
	{
		MDL.RelayPins[i] = NC;
	}
}

bool ValidData()
{
	bool Result = false;

	switch (Processor)
	{
	case 0:
		for (int i = 0; i < MDL.SensorCount; i++)
		{
			// flow pin
			Result = false;
			for (int j = 0; j < sizeof(ValidPins0); j++)
			{
				if (Sensor[i].FlowPin == ValidPins0[j])
				{
					Result = true;
					break;
				}
			}
			if (!Result) break;

			// IN1
			Result = false;
			for (int j = 0; j < sizeof(ValidPins0); j++)
			{
				if (Sensor[i].IN1 == ValidPins0[j])
				{
					Result = true;
					break;
				}
			}
			if (!Result) break;

			// IN2
			Result = false;
			for (int j = 0; j < sizeof(ValidPins0); j++)
			{
				if (Sensor[i].IN2 == ValidPins0[j])
				{
					Result = true;
					break;
				}
			}
			if (!Result) break;
		}

		if (Result && MDL.RelayControl == 1)
		{
			// check GPIOs for relays
			for (int k = 0; k < 16; k++)
			{
				Result = false;
				for (int j = 0; j < sizeof(ValidPins0); j++)
				{
					if ((MDL.RelayPins[k] == ValidPins0[j]) ||
						(MDL.RelayPins[k] == NC))
					{
						Result = true;
						break;
					}
				}
				if (!Result) break;
			}
		}
		break;
	}
	GoodPins = Result;
	return Result;
}

String scanI2CDevices(){
  String forReturn="";
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");   /*ESP32 starts scanning available I2C devices*/
  forReturn += "Scanning...\n";
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {   /*for loop to check number of devices on 127 address*/
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {   /*if I2C device found*/
      Serial.print("I2C device found at address 0x");/*print this line if I2C device found*/
      forReturn += "I2C device found at address 0x";
      if (address<16) {
        Serial.print("0");
        forReturn += "0";
      }
      Serial.println(address,HEX);  /*prints the HEX value of I2C address*/
      forReturn += String(address, HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknown error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n"); /*If no I2C device attached print this message*/
    forReturn += "No I2C devices found\n";
  }
  else {
    Serial.println("done\n");
  }
  return forReturn;
}

void initTempSensor(){
    temp_sensor_config_t temp_sensor = TSENS_CONFIG_DEFAULT();
    temp_sensor.dac_offset = TSENS_DAC_L2;  // TSENS_DAC_L2 is default; L4(-40°C ~ 20°C), L2(-10°C ~ 80°C), L1(20°C ~ 100°C), L0(50°C ~ 125°C)
    temp_sensor_set_config(temp_sensor);
    temp_sensor_start();
}

float getCurrentInAmps(int pin) {
  int volt = analogRead(pin);
  return map(volt,3000,500,0,30)/10.0;
}
