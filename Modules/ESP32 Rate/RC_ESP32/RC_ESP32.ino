#include <Wire.h>
#include "PCA95x5_RC.h"		// modified from https://github.com/hideakitai/PCA95x5

#include <Adafruit_MCP23008.h>
#include <Adafruit_MCP23X08.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_MCP23XXX.h>
#include <Adafruit_PWMServoDriver.h>

#include <Adafruit_BusIO_Register.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <WiFiAP.h>

#include "ETHClass.h"
#include <EthernetUdp.h>

#include <ArduinoOTA.h>
#include <WebServer.h>
#include <EEPROM.h> 

#include <elapsedMillis.h>
#include <esp_task_wdt.h>
#define WDT_TIMEOUT 90
#include "driver/temp_sensor.h"

#include <FastLED.h>
#define CRGB_LED_COUNT 4
#define CRGB_PIN 39
CRGB leds[4];

// rate control with ESP32	board: DOIT ESP32 DEVKIT V1
# define InoDescription "RC_ESP32 :  26-Dec-2023"
const uint16_t InoID = 26123;	// change to send defaults to eeprom, ddmmy, no leading 0

#define MaxReadBuffer 100	// bytes
#define MaxProductCount 1
#define EEPROM_SIZE 512
#define ModStringLengths 20

elapsedMicros SlowestStepCounter;
uint32_t SlowestStep;
String SlowestStepName;


struct ModuleConfig
{
	uint8_t ID = 0;
	uint8_t SensorCount = 2;        // up to 2 sensors, if 0 rate control will be disabled
	uint8_t RelayOnSignal = 0;	    // value that turns on relays
	uint8_t FlowOnDirection = 0;	// sets on value for flow valve or sets motor direction
	uint8_t IP0 = 192;
	uint8_t IP1 = 168;
	uint8_t IP2 = 1;
	uint8_t IP3 = 60;
	uint8_t RelayControl = 6;		// 0 - no relays, 1 - RS485, 2 - PCA9555 8 relays, 3 - PCA9555 16 relays, 4 - MCP23017, 5 - Teensy GPIO, 6 - PCA9685
	uint8_t RelayPins[16] = { 8,9,10,11,12,25,26,27,0,0,0,0,0,0,0,0 };		// pin numbers when GPIOs are used for relay control (5), default RC11
	char Name[ModStringLengths] = "RateModule";
	char SSID[ModStringLengths] = "tractor";
	char Password[ModStringLengths] = "111222333";
	uint8_t AdsAddress = 0x48;			// enter 0 to search all
};

ModuleConfig MDL;

struct SensorConfig
{
	uint8_t FlowPin;
	uint8_t DirPin;
	uint8_t PWMPin;
	bool FlowEnabled;
	double UPM;				// sent as upm X 1000
	double PWM;
	uint32_t CommTime;
	byte ControlType;		// 0 standard, 1 combo close, 2 motor, 3 motor/weight, 4 fan, 5 timed combo
	uint32_t TotalPulses;
	double TargetUPM;
	double MeterCal;
	double ManualAdjust;
	double KP;
	double KI;
	double KD;
	byte MinPWM;
	byte MaxPWM;
	bool UseMultiPulses;	// 0 - time for one pulse, 1 - average time for multiple pulses
	uint8_t Debounce;
};

SensorConfig Sensor[2];

// network
static bool ETHconnected = false; //Ethernet.linkStatus() is too slow
const uint16_t ListeningPort = 28888;
const uint16_t DestinationPort = 29999;

// ethernet
WiFiUDP UDPcomm;
IPAddress DestinationIP(MDL.IP0, MDL.IP1, MDL.IP2, 255);

// AGIO
WiFiUDP AGIOcomm;
uint16_t ListeningPortAGIO = 8888;		// to listen on
uint16_t DestinationPortAGIO = 9999;	// to send to

// wifi
WiFiUDP WifiComm;
IPAddress WifiDestinationIP(MDL.IP0, MDL.IP1, MDL.IP2, 255);
bool HardwareFound;
bool ESPconnected;
int8_t WifiStrength;
WiFiClient client;
WebServer server(80);

// control page
bool WifiMasterOn = false;
bool Button[16];
uint32_t WifiSwitchesTimer;

// Relays
byte RelayLo = 0;	// sections 0-7
byte RelayHi = 0;	// sections 8-15
byte PowerRelayLo;
byte PowerRelayHi;

const uint16_t LoopTime = 50;      //in msec = 20hz
uint32_t LoopLast = LoopTime;
const uint16_t SendTime = 200;
uint32_t SendLast = SendTime;

bool MasterOn = false;
bool AutoOn = true;

PCA9555 PCA;
bool PCA9555PW_found = false;

Adafruit_MCP23X17 MCP;
bool MCP23017_found = false;

Adafruit_PWMServoDriver PCA1 = Adafruit_PWMServoDriver(0x40);
bool PCA9685_1_found = false;

int TimedCombo(byte, bool);	// function prototype

// analog
struct AnalogConfig
{
	int16_t AIN0;	// Pressure 0
	int16_t AIN1;	// Pressure 1
	int16_t AIN2;
	int16_t AIN3;
};
AnalogConfig AINs;

int ADS1115_Address;
bool ADSfound = false;

void setup()
{
  SlowestStep = 0;
	DoSetup();
}

void loop()
{
	if (millis() - LoopLast >= LoopTime)
	{
		LoopLast = millis();

		for (int i = 0; i < MDL.SensorCount; i++)
		{
			Sensor[i].FlowEnabled = (millis() - Sensor[i].CommTime < 4000)
				&& ((Sensor[i].TargetUPM > 0 && MasterOn)
					|| ((Sensor[i].ControlType == 4) && (Sensor[i].TargetUPM > 0))
					|| (!AutoOn && MasterOn));
		}
    SlowestStepCounter = 0;
		CheckRelays();
//    SlowestStepHelper("CheckRelays");
		GetUPM();
    SlowestStepHelper("GetUPM");
		AdjustFlow();
    SlowestStepHelper("AdjustFlow");
		ReadAnalog();
    SlowestStepHelper("ReadAnalog");
		CheckWifi();
//    SlowestStepHelper("CheckWifi");
	}

	if (millis() - SendLast > SendTime)
	{
		SendLast = millis();
    SlowestStepCounter = 0;
		SendUDP();
    SlowestStepHelper("SendUDP");
	}

  SlowestStepCounter = 0;
	SetPWM();
  SlowestStepHelper("SetPWM");
	ReceiveUDP();
  SlowestStepHelper("ReceiveUDP");
	ReceiveAGIO();
  SlowestStepHelper("ReceiveAGIO");


	ArduinoOTA.handle();
  SlowestStepHelper("ArduinoOTA");
	server.handleClient();
	Blink();
  wdt_timer();
}

void SlowestStepHelper(String stepName){
  if(SlowestStep < SlowestStepCounter){
    SlowestStepName = stepName;
    SlowestStep = SlowestStepCounter;
  }
  SlowestStepCounter = 0;
}

byte ParseModID(byte ID)
{
	// top 4 bits
	return ID >> 4;
}

byte ParseSenID(byte ID)
{
	// bottom 4 bits
	return (ID & 0b00001111);
}

byte BuildModSenID(byte Mod_ID, byte Sen_ID)
{
	return ((Mod_ID << 4) | (Sen_ID & 0b00001111));
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
elapsedMillis BlinkTmr;
elapsedMicros LoopTmr;
elapsedMillis WdtTmr;
byte ReadReset;
uint32_t MaxLoopTime;

void wdt_timer() {
  // resetting WDT every 2s, 5 times only
  if (WdtTmr >= 5000) {
      WdtTmr = 0;
      Serial.println("Resetting WDT...");
      esp_task_wdt_reset();
      }
}

void Blink()
{
	if (BlinkTmr > 1000)
	{
		BlinkTmr = 0;
		State = !State;
		//digitalWrite(LED_BUILTIN, State);
		Serial.println(".");	// needed to allow PCBsetup to connect

		Serial.print(" Micros: ");
		Serial.print(MaxLoopTime);

		Serial.print(", Temp: ");
    float result = 0;
    temp_sensor_read_celsius(&result);
    Serial.print(result);

		Serial.print(", TotalPulses: ");
		Serial.print(Sensor[0].TotalPulses);
		Serial.println("");

		if (ReadReset++ > 5)
		{
			ReadReset = 0;
			MaxLoopTime = 0;
		}
	}
	if (LoopTmr > MaxLoopTime) MaxLoopTime = LoopTmr;
	LoopTmr = 0;
}
