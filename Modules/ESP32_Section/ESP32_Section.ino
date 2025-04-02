#include <ESP2SOTA.h>		// https://github.com/pangodream/ESP2SOTA

#include <Adafruit_PWMServoDriver.h>

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include <WebServer.h>
#include <EEPROM.h> 
#include <Wire.h>

//#include <SPI.h>
//#include <Ethernet.h>
#include "ETHClass.h"
#include <EthernetUdp.h>

#include <elapsedMillis.h>
#include "driver/temp_sensor.h"

// rate control with ESP32	board: DOIT ESP32 DEVKIT V1
# define InoDescription "Machine_ESP32 :  24-Mar-2024"
const uint16_t InoID = 24034;	// change to send defaults to eeprom, ddmmy, no leading 0
const uint8_t InoType = 4;		// 0 - Teensy AutoSteer, 1 - Teensy Rate, 2 - Nano Rate, 3 - Nano SwitchBox, 4 - ESP Rate
const uint8_t Processor = 0;	// 0 - ESP32-Wroom-32U

#define MaxReadBuffer 100	// bytes
#define EEPROM_SIZE 512
#define ModStringLengths 20

// servo driver
//#define OutputEnablePin 27
#define PCAaddress 0x40
#define PCAExtaddress 0x41

#define W5500_SS 5	// W5500 SPI SS
#define NC 0xFF		// Pin not connected

#define Current1Pin 6 //CURRENT_SECTIONS
#define Current2Pin 14 //CURRENT_CYTRON

#define activeON true

struct ModuleConfig
{
	uint8_t IP0 = 192;
	uint8_t IP1 = 168;
	uint8_t IP2 = 5;
	uint8_t IP3 = 123;
	uint8_t AdsAddress = 0x48;			// enter 0 to search all

};

ModuleConfig MDL;

    //Variables for config - 0 is false  
    struct Config {
        uint8_t raiseTime = 2;
        uint8_t lowerTime = 4;
        uint8_t enableToolLift = 0;
        uint8_t isRelayActiveHigh = 0; //if zero, active low (default)

        uint8_t user1 = 0; //user defined values set in machine tab
        uint8_t user2 = 0;
        uint8_t user3 = 0;
        uint8_t user4 = 0;

    };  
  Config aogConfig;   //4 bytes

// network
static bool ETHconnected = false; //Ethernet.linkStatus() is too slow

// ethernet
WiFiUDP UDP_Ethernet;
IPAddress Ethernet_DestinationIP(MDL.IP0, MDL.IP1, MDL.IP2, 255);
bool ChipFound;

// AGIO
WiFiUDP UDP_AGIO;
uint16_t ListeningPortAGIO = 8888;		// to listen on
uint16_t DestinationPortAGIO = 9999;	// to send to


WebServer server(80);

    /*
    * Functions as below assigned to pins
    0: -
    1 thru 16: Section 1,Section 2,Section 3,Section 4,Section 5,Section 6,Section 7,Section 8,
                Section 9, Section 10, Section 11, Section 12, Section 13, Section 14, Section 15, Section 16,
    17,18    Hyd Up, Hyd Down,
    19 Tramline,
    20: Geo Stop
    21,22,23 - unused so far*/    
    uint8_t pin[] = { 1,2,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

    //read value from Machine data and set 1 or zero according to list
    uint8_t relayState[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

    //hello from AgIO
    uint8_t helloFromMachine[] = { 128, 129, 123, 123, 5, 0, 0, 0, 0, 0, 71 };


const uint16_t LoopTime = 50;      //in msec = 20hz
uint32_t LoopLast = LoopTime;
const uint16_t SendTime = 200;
uint32_t SendLast = SendTime;

    //Comm checks
    uint8_t watchdogTimer = 20; //make sure we are talking to AOG
    uint8_t serialResetTimer = 0; //if serial buffer is getting full, empty it

    bool isRaise = false, isLower = false;

    //Communication with AgOpenGPS
    int16_t temp, EEread = 0;

    //Parsing PGN
    bool isPGNFound = false, isHeaderFound = false;
    uint8_t pgn = 0, dataLength = 0, idx = 0;
    int16_t tempHeader = 0;

    //settings pgn
    uint8_t PGN_237[] = { 0x80,0x81, 0x7f, 237, 8, 1, 2, 3, 4, 0,0,0,0, 0xCC };
    int8_t PGN_237_Size = sizeof(PGN_237) - 1;

    //The variables used for storage
    uint8_t relayHi = 0, relayLo = 0, tramline = 0, uTurn = 0, hydLift = 0, geoStop = 0;
    float gpsSpeed;
    uint8_t raiseTimer = 0, lowerTimer = 0, lastTrigger = 0;

Adafruit_PWMServoDriver PWMServoDriver = Adafruit_PWMServoDriver(PCAaddress);
bool PCA9685_found = false;
Adafruit_PWMServoDriver PWMServoDriverExt = Adafruit_PWMServoDriver(PCAExtaddress);
bool PCA9685Ext_found = false;

uint8_t DisconnectCount;

void setup()
{
	DoSetup();
}

void loop()
{
	if (millis() - LoopLast >= LoopTime)
	{
		LoopLast = millis();
            //If connection lost to AgOpenGPS, the watchdog will count up 
            if (watchdogTimer++ > 250) watchdogTimer = 20;

            //clean out serial buffer to prevent buffer overflow
            if (serialResetTimer++ > 20)
            {
                while (Serial.available() > 0) Serial.read();
                serialResetTimer = 0;
            }

            if (watchdogTimer > 20)
            {
                if (activeON == aogConfig.isRelayActiveHigh) { 
                    relayLo = 255;
                    relayHi = 255;
                }
                else {
                    relayLo = 0;
                    relayHi = 0;
                }
            }

            //hydraulic lift

            if (hydLift != lastTrigger && (hydLift == 1 || hydLift == 2))
            {
                lastTrigger = hydLift;
                lowerTimer = 0;
                raiseTimer = 0;

                //50 msec per frame so 20 per second
                switch (hydLift)
                {
                    //lower
                case 1:
                    lowerTimer = aogConfig.lowerTime * 20;
                    break;

                    //raise
                case 2:
                    raiseTimer = aogConfig.raiseTime * 20;
                    break;
                }
            }

            //countdown if not zero, make sure up only
            if (raiseTimer)
            {
                raiseTimer--;
                lowerTimer = 0;
            }
            if (lowerTimer) lowerTimer--;

            //if anything wrong, shut off hydraulics, reset last
            if ((hydLift != 1 && hydLift != 2) || watchdogTimer > 10) //|| gpsSpeed < 2)
            {
                lowerTimer = 0;
                raiseTimer = 0;
                lastTrigger = 0;
            }

            if (aogConfig.isRelayActiveHigh)
            {
                isLower = isRaise = false;
                if (lowerTimer) isLower = true;
                if (raiseTimer) isRaise = true;
            }
            else
            {
                isLower = isRaise = true;
                if (lowerTimer) isLower = false;
                if (raiseTimer) isRaise = false;
            }

		CheckRelays();
	}

	if (millis() - SendLast > SendTime)
	{
		SendLast = millis();
		SendAGIO();
	}

	ReceiveAGIO();

	server.handleClient();

//	Blink();
}

bool GoodCRC(byte Data[], byte Length)
{
	byte ck = CRC(Data, Length - 1, 0);
	bool Result = (ck == Data[Length - 1]);
	return Result;
}

byte CRC(byte Chk[], byte Length, byte Start)
{
	byte Result = 0;
	int CK = 0;
	for (int i = Start; i < Length; i++)
	{
		CK += Chk[i];
	}
	Result = (byte)CK;
	return Result;
}

bool State = false;
uint32_t LastBlink;
uint32_t LastLoop;
byte ReadReset;
uint32_t MaxLoopTime;
double debug1;
double debug2;
double debug3;

void Blink()
{
	if (millis() - LastBlink > 1000)
	{
		LastBlink = millis();
		State = !State;
		//digitalWrite(LED_BUILTIN, State);

		Serial.print(" Micros: ");
		Serial.print(MaxLoopTime);
		Serial.print(", ");
		Serial.print(debug1);
		Serial.print(", ");
		Serial.print(debug2);
		Serial.print(", ");
		Serial.print(debug3);

		Serial.println("");

		if (ReadReset++ > 5)
		{
			ReadReset = 0;
			MaxLoopTime = 0;
		}
	}
	if (micros() - LastLoop > MaxLoopTime) MaxLoopTime = micros() - LastLoop;
	LastLoop = micros();
}
