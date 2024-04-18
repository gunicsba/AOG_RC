
// valid pins for each processor
uint8_t ValidPins0[] = { 0,2,4,13,14,15,16,17,21,22,25,26,27,32,33 };	// SPI pins 5,18,19,23 excluded for ethernet module


void DoSetup()
{
	uint8_t ErrorCount;

	Serial.begin(38400);
	delay(3000);
	Serial.println("");
	Serial.println("");
	Serial.println("");

	// eeprom
	EEPROM.begin(EEPROM_SIZE);
	LoadData();

	// I2C
  Wire.begin(8,18,400000);			// I2C on pins SDA 8, SCL 18, data rate to 400kHz
  scanI2CDevices();
  

	// ethernet 
	Serial.println("Starting Ethernet ...");
	MDL.IP3 = 123;
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
  
  // AGIO
	UDP_AGIO.begin(ListeningPortAGIO);

  Serial.println("Sensors setup ");

  Serial.println("Relays setup ");

	// Relays
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
	

	// web server
	Serial.println();
	Serial.println("Starting Web Server");
	server.on("/", HandleRoot);
	server.on("/page1", HandlePage1);
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

	delay(200);
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
    EEPROM.get(6, aogConfig);
    EEPROM.get(20, pin);
		EEPROM.get(110, MDL);

	} else {
    SaveData();
  }
}

void SaveData()
{
	// update stored data
	Serial.println("Updating stored settings.");
	EEPROM.put(0, InoID);
	EEPROM.put(4, InoType);
  EEPROM.put(6, aogConfig);
  EEPROM.put(20, pin);
	EEPROM.put(110, MDL);

  EEPROM.commit();
}

void LoadDefaults()
{
	Serial.println("Loading default settings.");
}

bool ValidData()
{
	return true;
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
