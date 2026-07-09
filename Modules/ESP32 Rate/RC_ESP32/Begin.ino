// valid pins for ESP32-S3 (excludes strapping pins 0/3/45/46, flash 26-32, SPI 35-38)
uint8_t ValidPins0[] = { 1,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,33,34,39,40,41,42,43,44,47,48 };

void DoSetup()
{
	uint8_t ErrorCount = 0;
	bool WheelMatch = false;

	Serial.begin(38400);
	delay(3000);
	Serial.println("");
	Serial.println("");
	Serial.println("");

	// eeprom
	EEPROM.begin(EEPROM_SIZE);
	LoadData();
	LoadNetworks();
	LoadBoardID();

	Serial.println("");
	Serial.println(InoDescription);

	// version
	uint16_t yr = InoID % 10 + 2020;
	uint16_t rest = InoID / 10;
	uint8_t mn = rest % 100;
	uint16_t dy = rest / 100;

	String fwVer;
	if (mn <= 12 && dy <= 31)
	{
		fwVer = "Firmware Version: v";
		fwVer += String(yr);
		fwVer += ".";
		if (mn < 10) fwVer += "0";
		fwVer += String(mn);
		fwVer += ".";
		if (dy < 10) fwVer += "0";
		fwVer += String(dy);
	}
	else
	{
		fwVer = "Firmware Version: invalid";
	}
	Serial.println(fwVer);

	Serial.print("Module ID: ");
	Serial.println(MDL.ID);
	Serial.println("");

	if (MDL.WorkPin < NC) pinMode(MDL.WorkPin, INPUT_PULLUP);
	if (MDL.SensorCount > MaxProductCount) MDL.SensorCount = MaxProductCount;

	// I2C (ESP32-S3: SDA=8, SCL=18)
	Wire.begin(8, 18, 400000);

	// ADS1115
	if (MDL.ADS1115Enabled)
	{
		Serial.print("Starting ADS1115 at address ");
		Serial.println(ADS1115_Address);
		while (!ADSfound)
		{
			Wire.beginTransmission(ADS1115_Address);
			Wire.write(0b00000000);	//Point to Conversion register
			Wire.endTransmission();
			ADSfound = (Wire.requestFrom(ADS1115_Address, 2) == 2);
			Serial.print(".");
			delay(500);
			if (ErrorCount++ > 10) break;
		}
		Serial.println("");
		if (ADSfound)
		{
			Serial.println("ADS1115 found.");
			Serial.println("");
		}
		else
		{
			Serial.println("ADS1115 not found.");
			Serial.println("ADS1115 disabled.");
			Serial.println("");
		}
	}

	// ethernet (WT5500 SPI)
	Serial.println("Starting Ethernet ...");
	MDLnetwork.IP3 = MDL.ID + 50;
	IPAddress LocalIP(MDLnetwork.IP0, MDLnetwork.IP1, MDLnetwork.IP2, MDLnetwork.IP3);
	IPAddress Gateway(MDLnetwork.IP0, MDLnetwork.IP1, MDLnetwork.IP2, 1);
	IPAddress Mask(255, 255, 255, 0);

	WT5500setup();

	// config static IP immediately
	if (ETH.config(LocalIP, Gateway, Mask) == false) {
		Serial.println("WT5500 Configuration failed.");
	} else {
		Serial.println("WT5500 Configuration success.");
	}

	int timeout = 10;
	while (!ETHconnected && --timeout >= 0) {
		Serial.print("Linkup:");
		Serial.print(ETH.linkUp());
		Serial.print(" Linkspeed:");
		Serial.print(ETH.linkSpeed());
		Serial.print(" LocalIP:");
		Serial.print(ETH.localIP());
		Serial.println("  Wait for network connect ...");
		delay(500);
	}

	Ethernet_DestinationIP = IPAddress(MDLnetwork.IP0, MDLnetwork.IP1, MDLnetwork.IP2, 255);	// update from saved data

	// UDP
	UDP_Ethernet.begin(ListeningPort);

	// sensors
	for (int i = 0; i < MDL.SensorCount; i++)
	{
		pinMode(Sensor[i].FlowPin, INPUT_PULLUP);
		pinMode(Sensor[i].IN1, OUTPUT);
		pinMode(Sensor[i].IN2, OUTPUT);

		switch (i)
		{
		case 0:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR0, RISING);
			break;
		case 1:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR1, RISING);
			break;
		case 2:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR2, RISING);
			break;
		case 3:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR3, RISING);
			break;
		case 4:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR4, RISING);
			break;
		case 5:
			attachInterrupt(digitalPinToInterrupt(Sensor[i].FlowPin), ISR5, RISING);
			break;
		}

		// pwm frequency change from default 5000 Hz to 490 Hz, required for some valves to work
		// ESP32 core 2.0.x: channel-based LEDC API
		ledcSetup(i * 2, PWM_FREQ, PWM_BITS);
		ledcAttachPin(Sensor[i].IN1, i * 2);
		ledcWrite(i * 2, 0);

		ledcSetup(i * 2 + 1, PWM_FREQ, PWM_BITS);
		ledcAttachPin(Sensor[i].IN2, i * 2 + 1);
		ledcWrite(i * 2 + 1, 0);

		if (Sensor[i].FlowPin == MDL.WheelSpeedPin) WheelMatch = true;
	}

	// wheel speed sensor
	if (MDL.WheelSpeedPin != NC && !WheelMatch)
	{
		pinMode(MDL.WheelSpeedPin, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(MDL.WheelSpeedPin), ISR_Speed, FALLING);
	}

	// Relays
	if (MDL.RemoteRelayControl > 0)
	{
		InitializeRelays(MDL.OnboardRelayControl, 7);
		InitializeRelays(MDL.RemoteRelayControl, -1);
	}
	else
	{
		InitializeRelays(MDL.OnboardRelayControl, 15);
	}

	// Wifi
	WiFi.mode(WIFI_MODE_APSTA);
	WiFi.disconnect(true);

	// Access Point
	Wifi_DestinationIP = IPAddress(192, 168, MDL.ID + 200, 255);
	IPAddress AP_LocalIP = IPAddress(192, 168, MDL.ID + 200, 1);
	IPAddress AP_GateWay = AP_LocalIP;
	IPAddress AP_Subnet(255, 255, 255, 0);

	uint64_t mac = ESP.getEfuseMac();
	uint32_t low32 = (uint32_t)(mac & 0xFFFFFFFF);

	char suffix[9]; // 8 hex + null
	sprintf(suffix, "%08X", low32);

	String AP = MDL.APname;
	AP += "_";
	AP += suffix;

	WiFi.softAPConfig(AP_LocalIP, AP_GateWay, AP_Subnet);
	if (strlen(MDL.APpassword) >= 8)
	{
		// WPA2-PSK
		WiFi.softAP(AP.c_str(), MDL.APpassword, 6, false, 4);
	}
	else
	{
		// Fallback: invalid WPA passphrase length -> force open
		WiFi.softAP(AP.c_str(), nullptr, 6, false, 4);
	}

	dnsServer.start(AP_DNS_PORT, "*", AP_LocalIP);

	UDP_Wifi.begin(ListeningPort);

	Serial.println("");
	Serial.print("Access Point name: ");
	Serial.println(AP);
	Serial.print("Settings Page IP: ");
	Serial.println(AP_LocalIP);

	// web server
	Serial.println();
	Serial.println("Starting Web Server");

	server.on("/", HandleRoot);
	server.on("/page1", HandlePage1);
	server.on("/page2", HandlePage2);
	server.on("/info", HandleInfo);
	server.on("/settings", HandleSettings);
	server.on("/ButtonPressed", ButtonPressed);
	server.onNotFound(HandleRoot);

	server.on("/generate_204", []() {server.send(204, "text/plain", "");	});
	server.on("/fwlink", []() { server.send(200, "text/plain", "OK"); });
	server.on("/hotspot-detect.html", HTTP_GET, []() { server.send(200, "text/html", "<html><body>Portal</body></html>"); });
	server.on("/ncsi.txt", HTTP_GET, []() { server.send(200, "text/plain", "Microsoft NCSI"); });

	// Register custom update page BEFORE ESP2SOTA so it takes priority (first registration wins)
	server.on("/update", HTTP_GET, []() {
		server.sendHeader("Connection", "close");
		server.send(200, "text/html", GetPageUpdate());
	});

	server.begin();

	/* INITIALIZE ESP2SOTA LIBRARY */
	ESP2SOTA.begin(&server);

	Serial.println("OTA started.");

	// Cytron motor enable pin (GPIO13)
	pinMode(13, OUTPUT);
	digitalWrite(13, HIGH);	// enabled by default

	// scan I2C bus
	scanI2CDevices();

	// internal temperature sensor
	initTempSensor();

	// wifi client mode
	if (MDLnetwork.WifiModeUseStation)
	{
		// connect to network
		delay(1000);
		WiFi.onEvent(WiFiStationConnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_CONNECTED);
		WiFi.onEvent(WiFiGotIP, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_GOT_IP);
		WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
		WiFi.begin(MDLnetwork.SSID, MDLnetwork.Password);
		Serial.println();
		Serial.println("Connecting to wifi network ...");
	}

	delay(1500);

	Serial.println("");
	Serial.print("Sensors enabled: ");
	Serial.println(MDL.SensorCount);
	Serial.println("");
	Serial.println("Sensor 1: ");
	Serial.print("Flow Pin: ");
	Serial.println(Sensor[0].FlowPin);
	Serial.print("IN1 Pin: ");
	Serial.println(Sensor[0].IN1);
	Serial.print("IN2 Pin: ");
	Serial.println(Sensor[0].IN2);

	Serial.println("");
	Serial.println("Sensor 2: ");
	Serial.print("Flow Pin: ");
	Serial.println(Sensor[1].FlowPin);
	Serial.print("IN1 Pin: ");
	Serial.println(Sensor[1].IN1);
	Serial.print("IN2 Pin: ");
	Serial.println(Sensor[1].IN2);

	Serial.println("");

	Serial.print("Work Switch Pin: ");
	if (MDL.WorkPin == NC)
	{
		Serial.println(F("Disabled"));
	}
	else
	{
		Serial.println(MDL.WorkPin);
	}

	Serial.print("Pressure Pin: ");
	if (MDL.PressurePin == NC)
	{
		Serial.println(F("Disabled"));
	}
	else
	{
		Serial.println(MDL.PressurePin);
	}

	Serial.print(F("Wheel Speed Pin: "));
	if (WheelMatch)
	{
		Serial.println(F("error, duplicate flow pin"));
	}
	else if (MDL.WheelSpeedPin == NC)
	{
		Serial.println(F("Disabled"));
	}
	else
	{
		Serial.println(MDL.WheelSpeedPin);
	}

	if (ADSfound)
	{
		Serial.println(F("ADS1115: Enabled "));
	}
	else
	{
		Serial.println(F("ADS1115: Disabled "));
	}

	if (GoodPins)
	{
		Serial.println(F("Pin configuration correct."));
	}
	else
	{
		Serial.println(F("Pin configuration not correct."));
	}

	if (MDL.Is3Wire)
	{
		Serial.println(F("Valves are 3 wire."));
	}
	else
	{
		Serial.println(F("Valves are 2 wire."));
	}

	// PID damper + per-sensor auto mode
	for (int i = 0; i < MaxProductCount; i++)
	{
		OscDamp[i] = 1.0f;
		LastAboveTarget[i] = false;
		AutoOn[i] = true;
	}

	Serial.println("");
	Serial.println("Finished setup.");
	Serial.println("");
}

void InitializeRelays(uint8_t Control, int8_t End)
{
	uint8_t ErrorCount;
	switch (Control)
	{
	case 1:
		// Relay GPIO Pins
		Serial.println("");
		Serial.println("Using GPIO pins for relays.");
		for (int i = 0; i <= End; i++)
		{
			if (MDL.RelayControlPins[i] < NC)
			{
				pinMode(MDL.RelayControlPins[i], OUTPUT);
			}
		}
		break;

	case 2:
	case 3:
		// PCA9555 I/O expander on default address 0x20
		Serial.println("");
		Serial.println("Starting PCA9555 I/O Expander for relays ...");
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
		Serial.println("");
		break;

	case 4:
		// MCP23017 I/O expander on 0x20, 0x21

		Serial.println("");
		Serial.println("Starting MCP23017 for relays ...");

		ErrorCount = 0;
		MCP23017address = 0x21;
		while (!MCP23017_found)
		{
			// RC12-3
			Serial.print(".");
			Wire.beginTransmission(0x21);
			MCP23017_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(500);
			if (ErrorCount > 5) break;
		}

		if (!MCP23017_found)
		{
			ErrorCount = 0;
			MCP23017address = 0x20;
			while (!MCP23017_found)
			{
				Serial.print(".");
				Wire.beginTransmission(MCP23017address);
				MCP23017_found = (Wire.endTransmission() == 0);
				ErrorCount++;
				delay(500);
				if (ErrorCount > 5) break;
			}
		}

		Serial.println("");
		if (MCP23017_found)
		{
			Wire.beginTransmission(MCP23017address);
			Wire.write(0x00); // IODIRA register
			Wire.write(0x00); // set all of port A to outputs
			Wire.endTransmission();

			Wire.beginTransmission(MCP23017address);
			Wire.write(0x01); // IODIRB register
			Wire.write(0x00); // set all of port B to outputs
			Wire.endTransmission();

			Serial.println("MCP23017 found.");
		}
		else
		{
			Serial.println("MCP23017 not found.");
		}
		break;

	case 5:
		// PCA9685
		Serial.println("");
		Serial.println("Starting PCA9685 I/O Expander for relays ...");
		ErrorCount = 0;
		while (!PCA9685_found)
		{
			Serial.print(".");
			Wire.beginTransmission(PCA9685Address);
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
		}
		else
		{
			Serial.println("PCA9685 expander not found.");
		}

		// PCA9685 extended (for relays 8-15)
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

		if (PCA9685Ext_found)
		{
			Serial.println("PCA9685 extended expander found.");
			PWMServoDriverExt.begin();
			PWMServoDriverExt.setPWMFreq(200);
		}
		else
		{
			Serial.println("PCA9685 extended not found.");
		}
		Serial.println("");
		break;

	case 6:
		// PCF8574
		Serial.println("");
		Serial.println("Starting PCF8574 I/O Expander for relays ...");
		ErrorCount = 0;
		while (!PCF_found)
		{
			Serial.print(".");
			Wire.beginTransmission(PCF8574address);
			PCF_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(500);
			if (ErrorCount > 5) break;
		}

		Serial.println("");
		if (PCF_found)
		{
			Serial.println("PCF8574 found.");
			PCF.begin();
		}
		else
		{
			Serial.println("PCF8574 not found.");
		}
		break;
	}
}

// eeprom map:
// ID			0-1
// module type	2
// feature flags	10-12 (disableMotor, disableFlow, b9threlay)
// module data	23-147
// network		168-232
// sensor 1		253-356
// sensor 2		377-480

void LoadData()
{
	bool IsValid = false;
	int16_t StoredID;
	int8_t StoredType;
	EEPROM.get(0, StoredID);
	EEPROM.get(2, StoredType);
	if (StoredID == InoID && StoredType == InoType)
	{
		// load stored data
		Serial.println("Loading stored settings.");
		EEPROM.get(23, MDL);

		// feature flags
		disableMotor = EEPROM.read(10);
		disableFlow = EEPROM.read(11);
		b9threlay = EEPROM.read(12);

		for (int i = 0; i < MaxProductCount; i++)
		{
			EEPROM.get(253 + i * 124, Sensor[i]);
		}
		IsValid = ValidData();
	}

	if (!IsValid)
	{
		Serial.println("Stored settings not valid.");
		LoadDefaults();
		SaveData();
		GoodPins=true;
	}
}

void SaveData()
{
	Serial.println("Updating stored settings.");
	EEPROM.put(0, InoID);
	EEPROM.put(2, InoType);
	EEPROM.put(23, MDL);

	// feature flags
	EEPROM.write(10, disableMotor);
	EEPROM.write(11, disableFlow);
	EEPROM.write(12, b9threlay);

	for (int i = 0; i < MaxProductCount; i++)
	{
		EEPROM.put(253 + i * 124, Sensor[i]);
	}
	EEPROM.commit();
}

void LoadDefaults()
{
	Serial.println("Loading default settings.");

	// default flow pins (ESP32-S3)
	Sensor[0].FlowPin = 21;
	Sensor[0].IN1 = 4;
	Sensor[0].IN2 = 5;

	Sensor[1].FlowPin = 47;
	Sensor[1].IN1 = 7;
	Sensor[1].IN2 = 15;

	// default control settings
	for (int i = 0; i < 2; i++)
	{
		Sensor[i].MaxPWM = 255;
		Sensor[i].MinPWM = 5;
		Sensor[i].Kp = 45 / 100.0;	// Kp = 45 (KPdefault, app Props.cs) - matches uniform /100 decode (Receive.ino)
		Sensor[i].Ki = 50 / 100.0;	// Ki = 50 (KIdefault) - matches uniform /100 decode (Receive.ino)
		Sensor[i].Deadband = 0.015;
		Sensor[i].BrakePoint = 35;
		Sensor[i].PIDslowAdjust = 30;
		Sensor[i].SlewRate = 25;
		Sensor[i].MaxIntegral = 25;
		Sensor[i].TimedMinStart = 0.5;
		Sensor[i].TimedAdjust = 80;
		Sensor[i].TimedPause = 400;
		Sensor[i].PIDtime = 150;
		Sensor[i].PulseMin = 250;		// 4000 Hz
		Sensor[i].PulseMax = 1000000;	// 1 Hz
		Sensor[i].SampleWindow = 40;	// flow window: 40 centiseconds = 400 ms
	}

	// relay pins
	for (int i = 0; i < 16; i++)
	{
		MDL.RelayControlPins[i] = NC;
	}

	// module settings
	MDL.ID = 0;
	MDL.SensorCount = 2;
	MDL.InvertRelay = true;
	MDL.InvertFlow = true;
	MDL.OnboardRelayControl = 5;
	MDL.RemoteRelayControl = 0;
	MDL.WorkPin = NC;
	MDL.WorkPinIsMomentary = false;
	MDL.Is3Wire = true;
	MDL.ADS1115Enabled = true;
	MDL.PressurePin = NC;
	MDL.WheelCal = 0;
	MDL.WheelSpeedPin = NC;
}

bool ValidData()
{
	bool Result = false;

	switch (Processor)
	{
	case 0:
	case 1:
		// work switch
		Result = (MDL.WorkPin == NC);
		if (!Result)
		{
			for (int j = 0; j < sizeof(ValidPins0); j++)
			{
				if (MDL.WorkPin == ValidPins0[j])
				{
					Result = true;
					break;
				}
			}
			if (!Result) break;
		}

		// pressure pin
		if (Result && MDL.PressurePin < NC)
		{
			for (int j = 0; j < sizeof(ValidPins0); j++)
			{
				if (MDL.PressurePin == ValidPins0[j])
				{
					Result = true;
					break;
				}
			}
			if (!Result) break;
		}

		// wheel speed pin
		if (Result && MDL.WheelSpeedPin < NC)
		{
			for (int j = 0; j < sizeof(ValidPins0); j++)
			{
				if (MDL.WheelSpeedPin == ValidPins0[j])
				{
					Result = true;
					break;
				}
			}
			if (!Result) break;
		}

		if (Result)
		{
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
		}

		if (Result && MDL.OnboardRelayControl == 1)
		{
			// check GPIOs for relays
			for (int k = 0; k < 16; k++)
			{
				Result = false;
				for (int j = 0; j < sizeof(ValidPins0); j++)
				{
					if ((MDL.RelayControlPins[k] == ValidPins0[j])
						|| (MDL.RelayControlPins[k] == NC))
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

void LoadNetworks()
{
	ModuleNetwork tmp;
	EEPROM.get(168, tmp);
	if (tmp.Identifier == 9876)
	{
		MDLnetwork = tmp;
	}
	else
	{
		// load network defaults
		MDLnetwork.Identifier = 9876;
		MDLnetwork.IP0 = 192;
		MDLnetwork.IP1 = 168;
		MDLnetwork.IP2 = 1;
		MDLnetwork.IP3 = 50;
		MDLnetwork.WifiModeUseStation = false;
		strcpy(MDLnetwork.SSID, "Tractor");
		strcpy(MDLnetwork.Password, "111222333");

		SaveNetworks();
	}
}

void SaveNetworks()
{
	EEPROM.put(168, MDLnetwork);
	EEPROM.commit();
}

void scanI2CDevices()
{
	Serial.println("Scanning I2C bus...");
	byte count = 0;
	for (byte addr = 1; addr < 127; addr++)
	{
		Wire.beginTransmission(addr);
		if (Wire.endTransmission() == 0)
		{
			Serial.print("  Found device at 0x");
			if (addr < 16) Serial.print("0");
			Serial.println(addr, HEX);
			count++;
		}
	}
	Serial.print("  ");
	Serial.print(count);
	Serial.println(" device(s) found.");
}

// ESP32-S3 internal temperature sensor (ESP-IDF 4.4 / core 2.0.x API)
static temp_sensor_config_t temp_sensor_config = TSENS_CONFIG_DEFAULT();

void initTempSensor()
{
	temp_sensor_set_config(temp_sensor_config);
	temp_sensor_start();
}

float getChipTempC()
{
	float temp = 0;
	temp_sensor_read_celsius(&temp);
	return temp;
}

float getCurrentInAmps(int pin, float maxAmps)
{
	// ESP32-S3 ADC: 12-bit, 0-3.3V range
	int raw = analogRead(pin);
	float voltage = (raw / 4095.0f) * 3.3f;
	// Sensor outputs 2.5V at 0A, 0V at maxAmps
	float sensitivity = maxAmps / 2.5f;
	return (2.5f - voltage) * sensitivity;
}

void LoadBoardID()
{
	// Independent EEPROM slot, NOT guarded by InoID, so the board label survives a firmware
	// reflash / LoadDefaults (it identifies the hardware, not the settings). SaveData()
	// only writes offsets 0/2/23/253+, never the 3-22 region, so this is left untouched there.
	BoardLabel tmp;
	EEPROM.get(EE_BoardID, tmp);
	if (tmp.Identifier == BoardIDMagic)
	{
		MDLboard = tmp;
	}
	else
	{
		// uninitialized EEPROM -> start with an empty label
		MDLboard.Identifier = BoardIDMagic;
		memset(MDLboard.Text, 0, sizeof(MDLboard.Text));
		SaveBoardID();
	}
}

void SaveBoardID()
{
	MDLboard.Identifier = BoardIDMagic;
	EEPROM.put(EE_BoardID, MDLboard);
	EEPROM.commit();
}



