# Fork Changes Documentation: ESP32 Rate Controller

> Base commit: `465c5cbbf251d56972e12e5c983ac3a86f9df81b`
> Purpose: Document all changes made to the upstream ESP32 Rate Controller code so they can be re-implemented on a fresh fork.
>
> **Note:** Section 6 (PID Logic Changes) from the original fork was **NOT implemented** — the original PID system is preserved.

---

## 1. Hardware Platform: ESP32 → ESP32-S3

The most fundamental change is migrating from an ESP32 (DOIT DEVKIT V1) to an **ESP32-S3** based custom board. This requires:

- **ESP32-S3 Arduino Core** v2.0.x (tested with 2.0.14)
- Board selection in Arduino IDE: "ESP32S3 Dev Module" (`esp32:esp32:esp32s3`)
- Different pin assignments throughout (see Section 4)
- `Processor` constant changed from `0` to `1`

---

## 2. Library Changes

| Original | New | Reason |
|----------|-----|--------|
| `#include <pcf8574.h>` | `#include <PCF8574.h>` | Case-sensitive rename; using Arduino Library Manager version |
| `#include <SPI.h>` | *(removed)* | SPI no longer used directly (handled by ETHClass) |
| `#include <Ethernet.h>` | `#include "ETHClass.h"` | Custom Ethernet class for ESP32-S3 SPI Ethernet (WT5500) |
| `#include <EthernetUdp.h>` | *(removed)* | Not needed with WiFiUDP |
| *(none)* | `#include <elapsedMillis.h>` | Added for timing |
| *(none)* | `#include "driver/temp_sensor.h"` | ESP32-S3 internal temperature sensor (ESP-IDF 4.4 API) |
| `EthernetUDP` | `WiFiUDP` | UDP class type changed (ESP32-S3 ETH stack uses WiFiUDP for Ethernet) |
| `#include <Adafruit_GenericDevice.h>` | *(removed)* | Requires Adafruit BusIO 1.17+; not needed for our hardware |

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

> **⚠️ CRITICAL — Init Order on ESP32-S3:**
> On ESP32-S3 the `WiFi.onEvent()` callback for ETH events (enum 18–23) may not fire reliably.
> You **must** call `ETH.config()` **immediately** after `WT5500setup()`, **before** waiting for the link.
> Then poll `ETH.linkUp()` with a timeout loop — do **not** gate `ETH.config()` behind the `ETHconnected` boolean.
> Getting this order wrong results in no static IP being assigned and the web UI being unreachable.

```cpp
WT5500setup();

// 1. Set static IP IMMEDIATELY (do not wait for ETHconnected)
if (ETH.config(LocalIP, Gateway, Mask) == false) {
    Serial.println("WT5500 Configuration failed.");
} else {
    Serial.println("WT5500 Configuration success.");
}

// 2. Poll linkUp() with timeout
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

// 3. Start UDP
UDP_Ethernet.begin(ListeningPort);
```

**Link Status:** The event-driven boolean `ETHconnected` is set/cleared by the `WiFiEvent` handler in `WT5500.ino` when ETH events do fire. It is used in `Send.ino` and `Receive.ino` as the primary link indicator, but the init sequence must **not** depend on it.

---

## 4. Pin Mapping Changes (ESP32-S3 Board)

### Valid Pins Array
```cpp
// Excludes strapping pins 0/3/45/46, flash 26-32, SPI 35-38
uint8_t ValidPins0[] = { 1,2,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,33,34,39,40,41,42,43,44,47,48 };
```

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
| OutputEnablePin | GPIO 27 | *(removed)* |
| Ext Address | *(none)* | `0x41` (second PCA9685 for relays 8–15) |

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
| WorkPin | 40 | Hardcoded work switch pin (safe, not a strapping pin) |

---

## 5. Default Configuration Changes (ModuleConfig)

| Field | Original | New |
|-------|----------|-----|
| `SensorCount` | 1 | **2** |
| `ADS1115Enabled` | *(varied)* | **true** (always) |
| `OnboardRelayControl` | *(varied)* | **5** (PCA9685, forced) |
| `RemoteRelayControl` | *(varied)* | **5** (PCA9685, forced) |
| `WorkPin` | NC | **40** (hardcoded) |
| `PressurePin` | *(varied)* | **NC** (hardcoded) |

### Hardcoded Values (in `setup()`)
After `DoSetup()` loads EEPROM data, these are force-set to prevent app misconfiguration:
```cpp
MDL.OnboardRelayControl = 5;   // PCA9685
MDL.RemoteRelayControl = 5;    // PCA9685
MDL.ADS1115Enabled = true;
MDL.WorkPin = 40;
MDL.PressurePin = NC;
```

---

## ~~6. PID Logic Changes (`PID.ino`)~~ (NOT IMPLEMENTED)

> The PID logic changes from the original fork were **excluded** from this implementation. The original PID system in `PID.ino` is preserved as-is.
>
> For reference, here is what the fork changed (none of this is in the current code):

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
- **New function:** `String HtmlGetHead(String title)` — generates the shared HTML `<head>` with all CSS styles (button-72, button-on, button-off, InputCell, etc.)
- Previously, every page (`PgStart.ino`, `PgSwitches.ino`, `PgNetwork.ino`) had ~90 lines of duplicated HTML/CSS header code
- All three pages now call `HtmlGetHead("Page Name")` instead
- `PgNetwork.ino` retains page-specific CSS for checkboxes and form controls

### New Info Page (`PGInfo.ino`)
- **New file:** `PGInfo.ino` — serves `/info` route
- Displays:
  - ESP32-S3 core temperature (via `getChipTempC()`)
  - Total pulse count for Sensor[0] and Sensor[1]
  - Module config (ID, SensorCount, InvertRelay, InvertFlow, ControlType, OnboardRelay)
  - Relay values (RelayLo, RelayHi)
  - Sensor connected / Applying / PID enabled status
  - PWM values and UPM readings for both sensors
  - TargetUPM for both sensors
  - Current draw in Amps (Cytron and Sections via `getCurrentInAmps()`)
  - Ethernet status and WiFi AP/Station IPs
  - Three toggle checkboxes for feature flags (see Section 8)
  - Save button that POSTs to `/settings`

### New Web Routes (in `Begin.ino`)
```cpp
server.on("/info", HandleInfo);       // Info/diagnostics page
server.on("/settings", HandleSettings); // Feature flag save handler
```

### Main Page (`PgStart.ino`)
- Added link to Info page:
  ```cpp
  st += "      <p> <a class='button-72' href='/info' >Info</a> </p>";
  ```

---

## 8. New Feature Flags (EEPROM-persisted)

Three new boolean flags stored in EEPROM (addresses 10, 11, 12):

| Variable | EEPROM Addr | UI Label | Behavior |
|----------|-------------|----------|----------|
| `disableMotor` | 10 | "Disable Motor (8th relay)" | When true and Sensor[1] is motor (ControlType==2), GPIO 13 is set based on bit 7 of RelayLo — disables Cytron when master relay is off |
| `disableFlow` | 11 | "Disable Flow (8th relay)" | When true and bit 7 of RelayLo is set, forces Sensor UPM to 0 — stops flow when master is off |
| `b9threlay` | 12 | "9th relay controls S1 motor" | When true, 9th relay bit (bit 0 of NewHi) controls Sensor[1]'s PWM via `SetPWM(1, ...)` |

### Load/Save (in `Begin.ino`)
```cpp
// LoadData():
disableMotor = EEPROM.read(10);
disableFlow = EEPROM.read(11);
b9threlay = EEPROM.read(12);

// SaveData():
EEPROM.write(10, disableMotor);
EEPROM.write(11, disableFlow);
EEPROM.write(12, b9threlay);
```

### `/settings` Endpoint (in `GUI.ino`)
`HandleSettings()` handles form POST from Info page, reads checkbox states, updates flags, and calls `SaveData()` if changed.

---

## 9. Motor Control Changes (`Motor.ino`)

### b9threlay Guard
When `b9threlay` is active and processing Sensor[1], `AdjustFlow()` skips that sensor:
```cpp
if (b9threlay && i == 1) continue;
```

### LEDC Channel-Based PWM (ESP32 Core 2.0.x)
```cpp
// Begin.ino — setup uses channels (sensor i → channels i*2 and i*2+1):
ledcSetup(i * 2, PWM_FREQ, PWM_BITS);
ledcAttachPin(Sensor[i].IN1, i * 2);
ledcSetup(i * 2 + 1, PWM_FREQ, PWM_BITS);
ledcAttachPin(Sensor[i].IN2, i * 2 + 1);

// Motor.ino — SetPWM writes to channels, not pins:
ledcWrite(ID * 2, duty);      // IN1 channel
ledcWrite(ID * 2 + 1, 0);     // IN2 channel
```

---

## 10. Relay Control Changes (`Relays.ino`)

### PCA9685 PWM Values Fixed
```cpp
// Original (incorrect for on/off):
PWMServoDriver.setPWM(IOpin, 4096, 0);
PWMServoDriver.setPWM(IOpin + 1, 0, 4096);

// New (fully on / fully off):
Drv.setPWM(IOpin, 0, 4095);    // fully on
Drv.setPWM(IOpin + 1, 0, 0);   // fully off
```

### Extended PCA9685 Support (16 Sections)
When `PCA9685Ext_found` (address 0x41), relays 8–15 use `PWMServoDriverExt`:
```cpp
bool UseExt = (Start >= 8);
Adafruit_PWMServoDriver &Drv = UseExt ? PWMServoDriverExt : PWMServoDriver;
```

### Cytron Motor Disable via Relay
```cpp
if (disableMotor && Sensor[1].ControlType == Motor_ct)
    digitalWrite(13, bitRead(NewLo, 7));
```

### b9threlay — Front Motor via Relay 9
```cpp
if (b9threlay)
    SetPWM(1, bitRead(NewHi, 0) ? 255.0f : -255.0f);
```

---

## 11. Flow Sensor / Rate Changes (`Rate.ino`)

### ~~Debounce Timing~~ (NOT IMPLEMENTED)

> The fork changed the debounce multiplier from `×1000` to `×30`. This change was **not applied** — original debounce timing is preserved.
>
> For reference:
> ```cpp
> // Original:
> else if (dur > Sensor[0].Debounce * 1000)
> // Fork (NOT IMPLEMENTED):
> else if (dur > Sensor[0].Debounce * 30)
> ```

### disableFlow Feature
```cpp
if (disableFlow && bitRead(RelayLo, 7)) Sensor[i].UPM = 0;
```

---

## 12. UDP Communication Changes

### Link Status
All instances of `Ethernet.linkStatus() == LinkON` replaced with `ETHconnected` (in `Send.ino` and `Receive.ino`).

### PGN32700 Selective Parsing (`Receive.ino`)
The Module Configuration packet (PGN32700) is parsed selectively to protect the hardcoded hardware config:

**Accepted from app:**
- Module ID (`data[2]`)
- Sensor Count (`data[3]`)
- InvertRelay (command bit 0)
- InvertFlow (command bit 1)
- WorkPinIsMomentary (command bit 3)
- Is3Wire (command bit 4)

**Force-overridden after parsing:**
```cpp
MDL.ADS1115Enabled = true;
MDL.OnboardRelayControl = 5;  // PCA9685
MDL.RemoteRelayControl = 5;   // PCA9685
MDL.WorkPin = 40;
MDL.PressurePin = NC;
```

**Ignored entirely:** sensor pin assignments (data[7-12]), relay GPIO pins (data[13-28])

### ~~PGN CRC Check Disabled~~ (NOT IMPLEMENTED)

> The fork disabled CRC validation for incoming PGN 239 (machine control) packets. This change was **not applied** — CRC checks remain enabled.
>
> For reference:
> ```cpp
> // Original:
> if (len > PGNlength - 1)
> // Fork (NOT IMPLEMENTED):
> if (false && len > PGNlength - 1)
> ```
> Note: This was likely done for debugging in the fork and should be reviewed before use.

---

## 13. New Utility Functions (`Begin.ino`)

### `scanI2CDevices()`
Scans all 127 I2C addresses, prints found devices to Serial. Called during `DoSetup()`.

### `initTempSensor()`
Configures ESP32-S3 internal temperature sensor using ESP-IDF 4.4 API:
```cpp
static temp_sensor_config_t temp_sensor_config = TSENS_CONFIG_DEFAULT();
void initTempSensor() {
    temp_sensor_set_config(temp_sensor_config);
    temp_sensor_start();
}
```

### `getChipTempC()`
```cpp
float getChipTempC() {
    float temp = 0;
    temp_sensor_read_celsius(&temp);
    return temp;
}
```

### `getCurrentInAmps(int pin)`
Reads analog voltage from a current sense pin using ACS712-style conversion:
```cpp
float getCurrentInAmps(int pin) {
    int raw = analogRead(pin);
    float voltage = (raw / 4095.0f) * 3.3f;
    return (voltage - 2.5f) / 0.066f;  // 2.5V offset, 66mV/A sensitivity
}
```

---

## 14. New Global Variables (`RC_ESP32.ino`)

```cpp
bool ETHconnected = false;     // Event-driven Ethernet link status
bool disableMotor = false;     // Disable Cytron based on 8th relay
bool disableFlow = false;      // Zero UPM based on 8th relay
bool b9threlay = false;        // 9th relay controls S1 motor

const uint8_t Current1Pin = 6;     // Current sense — sections
const uint8_t Current2Pin = 14;    // Current sense — Cytron
#define PCAExtaddress 0x41         // Second PCA9685 address

Adafruit_PWMServoDriver PWMServoDriverExt = Adafruit_PWMServoDriver(PCAExtaddress);
bool PCA9685Ext_found = false;
```

---

## 15. ESP32 Core 2.0.x Compatibility Notes

The code is written for **ESP32 Arduino Core 2.0.x** (tested with 2.0.14). Key API differences from 3.x:

| Feature | Core 2.0.x API | Core 3.x API |
|---------|----------------|--------------|
| PWM setup | `ledcSetup(ch, freq, bits)` + `ledcAttachPin(pin, ch)` | `ledcAttach(pin, freq, bits)` |
| PWM write | `ledcWrite(channel, duty)` | `ledcWrite(pin, duty)` |
| Temp sensor | `temp_sensor_config_t`, `TSENS_CONFIG_DEFAULT()`, `temp_sensor_set_config()`, `temp_sensor_start()`, `temp_sensor_read_celsius()` | `temperature_sensor_config_t`, `TEMPERATURE_SENSOR_CONFIG_DEFAULT()`, `temperature_sensor_install()`, `temperature_sensor_enable()`, `temperature_sensor_get_celsius()` |

### `ValidData()` Fix
The `Processor` constant was changed to `1` for ESP32-S3, but `ValidData()` only had `case 0:` in its switch. Added `case 1:` as a fall-through so pin validation works correctly:
```cpp
switch (Processor) {
case 0:
case 1:   // ESP32-S3
    // validation logic...
```

### ESP2SOTA Sketch Structure
The local `ESP2SOTA_RC` files (`.h`, `.cpp`, `index_html.h`) must be in the **sketch root directory**, not in a subfolder. Arduino IDE only compiles `.cpp` files in the sketch root (and `src/`), not arbitrary subdirectories.

---

## 16. Summary of New Files

| File | Purpose |
|------|---------|
| `ETHClass.h` | Custom Ethernet class header for ESP32-S3 SPI Ethernet |
| `ETHClass.cpp` | Custom Ethernet class implementation (based on esp_eth) |
| `WT5500.ino` | WT5500 SPI Ethernet initialization and WiFiEvent handler |
| `PGInfo.ino` | New Info page with diagnostics and feature flag toggles |
| `ESP2SOTA_RC.h` | Local modified ESP2SOTA class header (in sketch root) |
| `ESP2SOTA_RC.cpp` | Local modified ESP2SOTA class implementation (in sketch root) |
| `index_html.h` | OTA update page HTML (in sketch root) |

---

## 17. Re-Implementation Checklist

When re-implementing these changes on a fresh fork:

- [ ] Switch board to ESP32-S3 (`Processor = 1`) and install ESP32 core 2.0.x
- [ ] Replace `#include <pcf8574.h>` with `#include <PCF8574.h>`
- [ ] Remove `#include <SPI.h>`, `#include <Ethernet.h>`, `#include <EthernetUdp.h>`, `#include <Adafruit_GenericDevice.h>`
- [ ] Add `#include <elapsedMillis.h>`, `#include "ETHClass.h"`, `#include "driver/temp_sensor.h"`
- [ ] Replace `EthernetUDP` with `WiFiUDP`
- [ ] Add `ETHClass.h`, `ETHClass.cpp`, `WT5500.ino` files
- [ ] Move `ESP2SOTA_RC.h`, `ESP2SOTA_RC.cpp`, `index_html.h` to sketch root (not subfolder)
- [ ] Update I2C pins: `Wire.begin(8, 18, 400000)`
- [ ] Update PCA9685 address from 0x55 to 0x40, remove OutputEnablePin usage
- [ ] Add PCAExtaddress (0x41) and PWMServoDriverExt
- [ ] Update flow sensor default pins in `LoadDefaults()`
- [ ] Add Current1Pin (6), Current2Pin (14), WorkPin (40) constants
- [ ] Replace W5500 init with WT5500setup() + ETH.config() in Begin.ino — **call `ETH.config()` immediately after `WT5500setup()`, THEN poll `linkUp()`** (see Section 3 warning)
- [ ] Add `ETHconnected` boolean and event handler in WT5500.ino
- [ ] Replace all `Ethernet.linkStatus()` checks with `ETHconnected`
- [ ] Extract HTML header into `HtmlGetHead()` in `GUI.ino`
- [ ] Create `PGInfo.ino` with diagnostics page
- [ ] Add `/info` and `/settings` web routes (NOT `/Cytron`)
- [ ] Add `disableMotor`, `disableFlow`, `b9threlay` flags with EEPROM read/write at addresses 10-12
- [ ] **Do NOT apply PID logic changes** (Section 6) — keep original PID.ino
- [ ] **Do NOT apply debounce timing change** (Section 11) — keep original ×1000 multiplier
- [ ] **Do NOT disable CRC check** (Section 12) — keep CRC validation enabled
- [ ] Fix PCA9685 PWM values (0,4095 / 0,0 instead of 4096,0 / 0,4096)
- [ ] Add PCA9685Ext relay support in Relays.ino case 5
- [ ] Add Cytron enable/disable via GPIO 13
- [ ] Add b9threlay guard in AdjustFlow()
- [ ] Add b9threlay relay-to-PWM logic in CheckRelays()
- [ ] Use LEDC channel-based API: `ledcSetup`/`ledcAttachPin`/`ledcWrite(channel, duty)`
- [ ] Use ESP-IDF 4.4 temp sensor API: `TSENS_CONFIG_DEFAULT()`, `temp_sensor_set_config()`, `temp_sensor_start()`, `temp_sensor_read_celsius()`
- [ ] Add `case 1:` fall-through in `ValidData()` switch for ESP32-S3
- [ ] Hardcode in setup(): OnboardRelay=5, RemoteRelay=5, ADS=true, WorkPin=40, PressurePin=NC
- [ ] Implement PGN32700 selective parsing (accept only InvertRelay/InvertFlow/WorkPinIsMomentary/Is3Wire; force PCA9685/ADS/WorkPin/PressurePin; ignore sensor/relay pins)
- [ ] Add disableFlow UPM override in Rate.ino
- [ ] Add utility functions: scanI2CDevices(), initTempSensor(), getChipTempC(), getCurrentInAmps()
- [ ] Change default SensorCount to 2
- [ ] Add Cytron enable pin setup: `pinMode(13, OUTPUT); digitalWrite(13, HIGH);`
