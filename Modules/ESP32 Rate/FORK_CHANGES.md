# Fork Changes Documentation: ESP32 Rate Controller

> Base commit: `465c5cbbf251d56972e12e5c983ac3a86f9df81b`
> Purpose: Document all changes made to the upstream ESP32 Rate Controller code so they can be re-implemented on a fresh fork.

---

## 1. Hardware Platform: ESP32 → ESP32-S3

The most fundamental change is migrating from an ESP32 (DOIT DEVKIT V1) to an **ESP32-S3** based custom board. This requires:

- **ESP32-S3 Arduino Core** (ESP SDK v2.x or v3.x — NOT the original ESP32 SDK)
- Board selection in Arduino IDE: an ESP32-S3 variant (e.g., "ESP32S3 Dev Module")
- Different pin assignments throughout (see Section 4)

---

## 2. Library Changes

| Original | New | Reason |
|----------|-----|--------|
| `#include <pcf8574.h>` | `#include <PCF8574.h>` | Case-sensitive rename; using Arduino Library Manager version |
| `#include <SPI.h>` | *(removed/commented)* | SPI no longer used directly (handled by ETHClass) |
| `#include <Ethernet.h>` | `#include "ETHClass.h"` | Custom Ethernet class for ESP32-S3 SPI Ethernet (WT5500) |
| *(none)* | `#include <elapsedMillis.h>` | Added for timing |
| *(none)* | `#include "driver/temp_sensor.h"` | ESP32-S3 internal temperature sensor |
| `EthernetUDP` | `WiFiUDP` | UDP class type changed (ESP32-S3 ETH stack uses WiFiUDP for Ethernet) |

---

## 3. Ethernet: W5500 → WT5500

### Original (W5500 with Arduino Ethernet library)
```cpp
Ethernet.init(W5500_SS);   // SS pin 5
Ethernet.begin(LocalMac, 0);
Ethernet.setLocalIP(LocalIP);
Ethernet.setSubnetMask(Mask);
Ethernet.setGatewayIP(Gateway);
ChipFound = (Ethernet.hardwareStatus() != EthernetNoHardware);
// link check: Ethernet.linkStatus() == LinkON
```

### New (WT5500 with custom ETHClass)

**New files added:**
- `ETHClass.h` — custom Ethernet wrapper based on ESP-IDF `esp_eth`
- `ETHClass.cpp` — implementation of `ETHClass` with SPI support and `config()`
- `WT5500.ino` — WT5500 SPI Ethernet setup and event handler

**WT5500 SPI Pin Definitions** (in `WT5500.ino`):
| Signal | GPIO |
|--------|------|
| MISO | 37 |
| MOSI | 35 |
| SCLK | 36 |
| CS | 38 |
| INT | 45 |
| RST | 48 |

**Initialization** (in `Begin.ino`, replaces the old Ethernet block):
```cpp
WT5500setup();
ETH.config(LocalIP, Gateway, Mask);
// Wait for ETHconnected with timeout
UDP_Ethernet.begin(ListeningPort);
UDP_AGIO.begin(ListeningPortAGIO);
```

**Link Status:** Replaced polling `Ethernet.linkStatus() == LinkON` with an event-driven boolean `ETHconnected` that is set/cleared by the `WiFiEvent` handler in `WT5500.ino`. This is used throughout `UDPComm.ino`.

---

## 4. Pin Mapping Changes (ESP32-S3 Board)

### I2C
| Signal | Original (ESP32) | New (ESP32-S3) |
|--------|-------------------|-----------------|
| SDA | 21 | **8** |
| SCL | 22 | **18** |

```cpp
// Original
Wire.begin();           // default SDA 21, SCL 22
Wire.setClock(400000);

// New
Wire.begin(8, 18, 400000);  // SDA 8, SCL 18, 400kHz
scanI2CDevices();            // new I2C scanner function
```

### PCA9685 Servo Driver
| Parameter | Original | New |
|-----------|----------|-----|
| Address | `0x55` | `0x40` |
| OutputEnablePin | GPIO 27 | *(removed — commented out)* |
| Ext Address | *(none)* | `0x41` (second PCA9685) |

### Flow Sensor Default Pins (in `LoadDefaults()`)
| Signal | Original Sensor[0] | New Sensor[0] | Original Sensor[1] | New Sensor[1] |
|--------|--------------------|---------------|--------------------|---------------|
| FlowPin | 17 | **21** | 16 | **47** |
| IN1 | 32 | **4** | 25 | **7** |
| IN2 | 33 | **5** | 26 | **15** |

### New Pins
| Name | GPIO | Purpose |
|------|------|---------|
| Current1Pin | 6 | Analog current sense — section relays |
| Current2Pin | 14 | Analog current sense — Cytron motor driver |
| Cytron enable | 13 | Digital output to enable/disable Cytron driver |

---

## 5. Default Configuration Changes (ModuleConfig)

| Field | Original | New |
|-------|----------|-----|
| `SensorCount` | 1 | **2** |
| `WifiMode` | 1 (Station + AP) | **0** (AP only) |

---

## 6. PID Logic Changes (`PID.ino`)

The PID valve control algorithm was modified for better stability:

### Key Changes in `PIDvalve()`:
1. **Integral anti-windup with direction detection:**
   ```cpp
   if ((RateError > 0) != (Result > 0)) IntegralSum[ID] = 0;
   ```
   Resets integral sum when error direction and output direction disagree (overshoot prevention).

2. **Integral sum uses absolute error and is capped:**
   ```cpp
   IntegralSum[ID] += Sensor[ID].KI * abs(RateError) * SF;
   if (IntegralSum[ID] > 50) IntegralSum[ID] = 100;
   ```

3. **Deadband resets integral:**
   ```cpp
   // Inside deadband:
   Result = 0;
   IntegralSum[ID] = 0;  // NEW: also reset integral
   ```

4. **PWM direction based on RateError, not Result:**
   ```cpp
   // Original
   bool IsPositive = (Result > 0);
   // New
   bool IsPositive = (RateError > 0);
   ```

5. **Proportional term uses absolute error:**
   ```cpp
   Result = Sensor[ID].MinPWM + Sensor[ID].KP * SF * abs(RateError) + IntegralSum[ID] + DifValue;
   ```

### New Debug Function:
`getDebugPID(byte ID)` — returns an HTML string with PID debug values (TargetUPM, UPM, RateError, IntegralSum, KP, KD, etc.) displayed on the Info page.

---

## 7. HTML/GUI Refactoring

### Extracted Common Header (`GUI.ino`)
- **New function:** `HtmlGetHead(String title)` — generates the shared HTML `<head>` with all CSS styles
- Previously, every page (`PgStart.ino`, `PgSwitches.ino`, `PgNetwork.ino`) had ~90 lines of duplicated HTML/CSS header code
- All three pages now call `HtmlGetHead("Wifi AOG")` instead
- Added `body { margin-top: 50px; background-color: wheat; }` and `a:link { font-size: 150%; }` styling

### New Info Page (`PGInfo.ino`)
- **New file:** `PGInfo.ino` — serves `/info` route
- Displays:
  - Loop cycle time (`MaxLoopTime`)
  - ESP32-S3 core temperature (via `temp_sensor_read_celsius`)
  - Total pulse count for Sensor[0]
  - Module config (ID, SensorCount, RelayControl, ControlType, FlowEnabled)
  - Relay values (RelayLo, RelayHi)
  - Current draw in Amps (via `getCurrentInAmps()` for both section and Cytron)
  - PID debug info for valve 0
  - Three toggle checkboxes (see Section 8)

### New Web Routes (in `Begin.ino`)
```cpp
server.on("/info", HandleInfo);
server.on("/Cytron", Cytron);
```

### Main Page Update (`PgStart.ino`, `PgNetwork.ino`)
- Added link to new Info page:
  ```cpp
  st += "      <p> <a class='button-72' href='/info' >Show Info page</a> </p>";
  ```

---

## 8. New Feature Flags (EEPROM-persisted)

Three new boolean flags stored in EEPROM (addresses 10, 11, 12):

| Variable | EEPROM Addr | UI Label | Behavior |
|----------|-------------|----------|----------|
| `disableMotor` | 10 | "Disable Motor drive based on 8th relay" | When true and Sensor[1] is motor (ControlType==2), GPIO 13 is set based on bit 7 of RelayLo (8th relay) — effectively disabling the Cytron motor driver when master relay is off |
| `disableFlow` | 11 | "Disable Flow value based on 8th relay" | When true and bit 8 of RelayLo is set, forces Sensor UPM to 0 — stops flow control when master is off |
| `b9threlay` | 12 | "9th relay controls F2" | When true, the 9th relay bit (bit 0 of NewHi) controls Sensor[1]'s PWM output via `SetPWM(1, ...)` — used for front motor control |

### Load/Save (in `Begin.ino`)
```cpp
// LoadData():
EEPROM.get(10, disableMotor);
EEPROM.get(11, disableFlow);
EEPROM.get(12, b9threlay);

// SaveData():
EEPROM.put(10, disableMotor);
EEPROM.put(11, disableFlow);
EEPROM.put(12, b9threlay);
```

### `/Cytron` Endpoint (in `GUI.ino`)
Handles form POST from Info page to update the three flags and save to EEPROM.

---

## 9. Motor Control Changes (`Motor.ino`)

### b9threlay Guard
When `b9threlay` is active and processing Sensor[1], `AdjustFlow()` returns early (flow is controlled by relay logic instead).

### PWM Off State Fix
```cpp
// Added: explicitly set PWM to 0 when flow is not enabled
if (Sensor[i].FlowEnabled) {
    SetPWM(i, Sensor[i].PWM);
} else {
    SetPWM(i, 0);  // NEW
}
```

### SetPWM Direction Logic Inverted
The original logic sent positive PWM to IN1 and negative to IN2. The new logic inverts this:
```cpp
// Original: PWM > 0 → IN1 active
// New: PWM < 0 → IN1 active (after FlowOnDirection multiplication)
if (PWM < 0) {
    PWM = abs(PWM);
    ledcWrite(ID * 2, PWM);     // IN1
    ledcWrite(ID * 2 + 1, 0);   // IN2
} else {
    ledcWrite(ID * 2 + 1, PWM); // IN2
    ledcWrite(ID * 2, 0);       // IN1
}
```

---

## 10. Relay Control Changes (`Relays.ino`)

### PCA9685 PWM Values Fixed
```cpp
// Original (incorrect for on/off):
PWMServoDriver.setPWM(IOpin, 4096, 0);
PWMServoDriver.setPWM(IOpin + 1, 0, 4096);

// New (fully on / fully off):
PWMServoDriver.setPWM(IOpin, 0, 4095);    // fully on
PWMServoDriver.setPWM(IOpin + 1, 0, 0);   // fully off
```

### Extended PCA9685 Support (16 Sections)
When `PCA9685Ext_found` (address 0x41), relays 8–15 are controlled via `PWMServoDriverExt` — same logic as the primary but offset by 8.

### Cytron Motor Disable via Relay
```cpp
if (disableMotor && Sensor[1].ControlType == 2)
    digitalWrite(13, bitRead(NewLo, 7));
```

### b9threlay — Front Motor via Relay 9
```cpp
if (b9threlay) {
    SetPWM(1, bitRead(NewHi, 0) ? 255 : -255);
}
```

---

## 11. Flow Sensor / Rate Changes (`Rate.ino`)

### Debounce Timing
```cpp
// Original: microseconds
else if (dur > Sensor[0].Debounce * 1000)

// New: shorter debounce (×30 instead of ×1000)
else if (dur > Sensor[0].Debounce * 30)
```

### disableFlow Feature
```cpp
if (disableFlow && bitRead(RelayLo, 8)) {
    Sensor[ID].UPM = 0;
}
```

---

## 12. UDP Communication Changes (`UDPComm.ino`)

All instances of:
```cpp
Ethernet.linkStatus() == LinkON
```
Replaced with:
```cpp
ETHconnected
```

### PGN CRC Check Disabled
```cpp
// Original:
if (len > PGNlength - 1)

// New (disabled):
if (false && len > PGNlength - 1)
```
> Note: This disables CRC validation for incoming PGN 239 (machine control) packets. This may have been done for debugging and should be reviewed.

---

## 13. New Utility Functions (`Begin.ino`)

### `scanI2CDevices()`
Scans all 127 I2C addresses, prints found devices to Serial, returns a formatted string. Called during `DoSetup()`.

### `initTempSensor()`
Configures ESP32-S3 internal temperature sensor with `TSENS_DAC_L2` range (-10°C to 80°C).

### `getCurrentInAmps(int pin)`
Reads analog voltage from a current sense pin and maps it to amps (0–3.0A range):
```cpp
float getCurrentInAmps(int pin) {
    int volt = analogRead(pin);
    return map(volt, 3000, 500, 0, 30) / 10.0;
}
```

---

## 14. New Global Variables (`RC_ESP32.ino`)

```cpp
static bool ETHconnected = false;  // Event-driven Ethernet link status
bool disableMotor = false;         // Disable Cytron based on 8th relay
bool disableFlow = false;          // Zero UPM based on 8th relay
bool b9threlay = false;            // 9th relay controls F2 motor

#define Current1Pin 6              // Current sense — sections
#define Current2Pin 14             // Current sense — Cytron
#define PCAExtaddress 0x41         // Second PCA9685 address

Adafruit_PWMServoDriver PWMServoDriverExt = Adafruit_PWMServoDriver(PCAExtaddress);
bool PCA9685Ext_found = false;
```

---

## 15. Summary of New Files

| File | Purpose |
|------|---------|
| `ETHClass.h` | Custom Ethernet class header for ESP32-S3 SPI Ethernet |
| `ETHClass.cpp` | Custom Ethernet class implementation (based on esp_eth) |
| `WT5500.ino` | WT5500 SPI Ethernet initialization and WiFiEvent handler |
| `GUI.ino` | Refactored GUI handlers + `HtmlGetHead()` + `/Cytron` endpoint |
| `PGInfo.ino` | New Info page with diagnostics and feature toggles |

---

## 16. Commit History Summary

| Commit | Description |
|--------|-------------|
| `7b224c07` | Updated libraries to Arduino Library Manager; refactored HTML pages to use shared header |
| `3c524cf4` | Updated code to match David's codebase; switched to custom ETHClass |
| `bbc0f53e` | Disabled pin override to avoid bootloop |
| `7a4e85cd` | Updated INO date |
| `dbbe9504` | Added support for +8 sections (PCA9685Ext) |
| `7b0b562a` | 2nd sensor support |
| `b994e957` | Front motor control (Árpád feature) |
| `bf421f99` | Published double sensor version |
| `dfaf6edd` | Added radio buttons for Info page |
| `27e2c546` | Atozz — 9th relay controls F2 |

---

## 17. Re-Implementation Checklist

When re-implementing these changes on a fresh fork:

- [ ] Switch board to ESP32-S3 and install appropriate Arduino core
- [ ] Replace `#include <pcf8574.h>` with `#include <PCF8574.h>`
- [ ] Add `#include <elapsedMillis.h>` and `#include "driver/temp_sensor.h"`
- [ ] Replace `EthernetUDP` with `WiFiUDP` for both UDP instances
- [ ] Comment out `#include <SPI.h>` and `#include <Ethernet.h>`
- [ ] Add `ETHClass.h`, `ETHClass.cpp`, `WT5500.ino` files
- [ ] Update I2C pins: `Wire.begin(8, 18, 400000)`
- [ ] Update PCA9685 address from 0x55 to 0x40, remove OutputEnablePin usage
- [ ] Add PCAExtaddress (0x41) and PWMServoDriverExt
- [ ] Update flow sensor default pins in `LoadDefaults()`
- [ ] Add Current1Pin (6) and Current2Pin (14)
- [ ] Replace W5500 init with WT5500setup() + ETH.config() in Begin.ino
- [ ] Add `ETHconnected` boolean and event handler in WT5500.ino
- [ ] Replace all `Ethernet.linkStatus()` checks with `ETHconnected`
- [ ] Extract HTML header into `HtmlGetHead()` in `GUI.ino`
- [ ] Create `PGInfo.ino` with diagnostics page
- [ ] Add `/info` and `/Cytron` web routes
- [ ] Add `disableMotor`, `disableFlow`, `b9threlay` flags with EEPROM load/save
- [ ] Apply PID logic changes (integral windup, abs error, direction fix)
- [ ] Fix PCA9685 PWM values (0,4095 / 0,0 instead of 4096,0 / 0,4096)
- [ ] Add PCA9685Ext relay support in Relays.ino case 6
- [ ] Add Cytron enable/disable via GPIO 13
- [ ] Invert SetPWM direction logic in Motor.ino
- [ ] Add PWM=0 when FlowEnabled is false
- [ ] Change debounce multiplier from 1000 to 30 in Rate.ino
- [ ] Add disableFlow UPM override in Rate.ino
- [ ] Add b9threlay guard in AdjustFlow()
- [ ] Add utility functions: scanI2CDevices(), initTempSensor(), getCurrentInAmps()
- [ ] Change default SensorCount to 2, WifiMode to 0
