
void DoSetup()
{
  fastLEDSetup();
	uint8_t ErrorCount;
	int ADS[] = { 0x48,0x49,0x4A,0x4B };	// ADS1115 addresses

	Sensor[0].FlowEnabled = false;
	Sensor[1].FlowEnabled = false;

	// default flow pins
	Sensor[0].FlowPin = 21;
	Sensor[0].DirPin = 4;
	Sensor[0].PWMPin = 5;

	// default flow pins
	Sensor[1].FlowPin = 47;
	Sensor[1].DirPin = 7;
	Sensor[1].PWMPin = 15;

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
	Sensor[1].MinPWM = 0;
	Sensor[1].MaxPWM = 255;
	Sensor[1].Debounce = 3;

	Serial.begin(38400);
	delay(3000);
	Serial.println("");
	Serial.println("");
	Serial.println("");
	Serial.println(InoDescription);
	Serial.println("");

	// eeprom
	EEPROM.begin(EEPROM_SIZE);

	int16_t StoredID;
	EEPROM.get(100, StoredID);
	if (StoredID == InoID)
	{
		LoadData();
	}
	else
	{
		SaveData();
	}

	if (MDL.SensorCount > MaxProductCount) MDL.SensorCount = MaxProductCount;

	Serial.println("");
	Serial.print("Module ID: ");
	Serial.println(MDL.ID);
	Serial.print("Module Version: ");
	Serial.println(InoID);
	Serial.println("");

//	ConnectWifi();
//	delay(3000);

  initTempSensor();
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
	MDL.IP3 = MDL.ID + 60;
	IPAddress LocalIP(MDL.IP0, MDL.IP1, MDL.IP2, MDL.IP3);
 	IPAddress gateway(MDL.IP0, MDL.IP1, MDL.IP2, 1);
  IPAddress subnet(255, 255, 255, 0);
	DestinationIP = IPAddress(MDL.IP0, MDL.IP1, MDL.IP2, 255);	// update from saved data
  static uint8_t LocalMac[] = { 0x0A,0x0B,0x42,0x0C,0x0D,MDL.IP3 };

  WT5500setup();

  // write confir for static IP, gateway,subnet,dns1,dns2
    if (ETH.config(LocalIP, gateway, subnet) == false) {
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

	
	Serial.println("");

	// UDP
	UDPcomm.begin(ListeningPort);

	// AGIO
	AGIOcomm.begin(ListeningPortAGIO);

	// sensors
	for (int i = 0; i < MDL.SensorCount; i++)
	{
		pinMode(Sensor[i].FlowPin, INPUT_PULLUP);
		pinMode(Sensor[i].DirPin, OUTPUT);
		pinMode(Sensor[i].PWMPin, OUTPUT);

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
		ledcSetup(i, 500, 8);
		ledcAttachPin(Sensor[i].PWMPin, i);
    ledcSetup(4+i, 500, 8);
		ledcAttachPin(Sensor[i].DirPin, 4+i);
	}

	// Relays
	switch (MDL.RelayControl)
	{
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
		Serial.println("");
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
		// Relay GPIO Pins
		for (int i = 0; i < 16; i++)
		{
			if (MDL.RelayPins[i] > 0)
			{
				pinMode(MDL.RelayPins[i], OUTPUT);
			}
		}
		break;
  case 6:
    // PCA9685 
    Serial.println("");
    Serial.println("Starting PCA9685 at address: 0x40");
		int ErrorCount = 0;
		while (!PCA9685_1_found)
		{
			Serial.print(".");
			Wire.beginTransmission(0x40);
			PCA9685_1_found = (Wire.endTransmission() == 0);
			ErrorCount++;
			delay(250);
			if (ErrorCount > 5) break;
		}
    if(PCA9685_1_found) {
      Serial.println("Found PCA Controller ...");
      PCA1.begin();
      PCA1.reset(); //TODO do we need this many begin and reset?
      PCA1.begin();
      PCA1.setPWMFreq(1600);
      Serial.print("PCA9685 init done ");
    } else {
      Serial.println("PCA Controller missing!!!");
    }
    Serial.println("");
    break;
	}

	StartOTA();

	String AP = MDL.Name;
	AP += "  (";
	AP += WiFi.macAddress();
	AP += ")";
	WiFi.softAP(AP);

	Serial.println("");
	Serial.print("Access Point name: ");
	Serial.println(AP);
	Serial.println("Access Point IP: 192.168.4.1");

	// web server
	Serial.println();
	Serial.println("Starting Web Server");
	server.on("/", HandleRoot);
	server.on("/page1", HandlePage1);
	server.on("/page2", HandlePage2);
  server.on("/info", HandleInfo);
	server.on("/ButtonPressed", ButtonPressed);
  server.on("/Cytron", Cytron);
	server.onNotFound(HandleRoot);
	server.begin();

  //watchdog timer
  Serial.println("WDT setup.");
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);

	Serial.println("");
	Serial.println("Finished setup.");
	Serial.println("");
}

void LoadData()
{
	// load stored data
	Serial.println("Loading stored settings.");
	EEPROM.get(110, MDL);

	for (int i = 0; i < MaxProductCount; i++)
	{
		EEPROM.get(300 + i * 80, Sensor[i]);
	}
}

void SaveData()
{
	// update stored data
	Serial.println("Updating stored data.");
	EEPROM.put(100, InoID);
	EEPROM.put(110, MDL);

	for (int i = 0; i < MaxProductCount; i++)
	{
		EEPROM.put(300 + i * 80, Sensor[i]);
	}
	EEPROM.commit();
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

void fastLEDSetup() {
  GPIO.func_out_sel_cfg[CRGB_PIN].inv_sel = 1;
  FastLED.addLeds<WS2812, CRGB_PIN, GRB>(leds, CRGB_LED_COUNT);
/*
  FastLED.clear();
  for(int i = 0; i < 255; i++){
    leds[1] = CRGB(i,0,0);
  FastLED.show();
    delay(50);
    leds[1] = CRGB(0,i,0);
      FastLED.show();
    delay(50);
    leds[1] = CRGB(0,0,i);
      FastLED.show();
    delay(50);
    Serial.println(i);
  }
  FastLED.clear();
*/
}