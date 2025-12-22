# Step-by-Step Manual Merge Resolution Guide

This guide provides the exact changes needed to resolve each conflict when merging master into ESP32RC_refactor25.

## Prerequisites

```bash
git checkout ESP32RC_refactor25
git merge master --no-commit --no-ff
# Conflicts will appear - follow steps below
```

## File 1: Modules/ESP32 Rate/Notes.txt

**Action:** Keep ESP32RC_refactor25 version (ours)

```bash
git checkout --ours "Modules/ESP32 Rate/Notes.txt"
git add "Modules/ESP32 Rate/Notes.txt"
```

**Reason:** Contains flashing instructions and library versions specific to ESP32RC_refactor25.

---

## File 2: Modules/ESP32 Rate/RC_ESP32.ino.bin

**Action:** Keep ESP32RC_refactor25 version (ours)

```bash
git checkout --ours "Modules/ESP32 Rate/RC_ESP32.ino.bin"
git add "Modules/ESP32 Rate/RC_ESP32.ino.bin"
```

**Reason:** Binary file needs rebuild after source changes anyway.

---

## File 3: Modules/ESP32 Rate/RC_ESP32/RC_ESP32.ino

**Conflict Location:** Line 23-27  
**Conflict:** InoID value

### Resolution:

Open the file in an editor and find this section:
```cpp
<<<<<<< HEAD
const uint16_t InoID = 20115;	// change to send defaults to eeprom, ddmmy, no leading 0
=======
const uint16_t InoID = 30115;	// change to send defaults to eeprom, ddmmy, no leading 0
>>>>>>> master
```

Replace the entire conflict block with:
```cpp
const uint16_t InoID = 30115;	// change to send defaults to eeprom, ddmmy, no leading 0
```

Then stage the file:
```bash
git add "Modules/ESP32 Rate/RC_ESP32/RC_ESP32.ino"
```

**Reason:** Use the newer version number from master (30115 vs 20115).

---

## File 4: Modules/ESP32 Rate/RC_ESP32/Begin.ino

This file has TWO conflicts.

### Conflict 1: ValidPins Array (Line 2-6)

**Current conflict:**
```cpp
// valid pins for each processor
<<<<<<< HEAD
uint8_t ValidPins0[] = { 0,2,4,5,7,13,14,15,16,17,21,22,25,26,27,32,33,47 };	// SPI pins 5,18,19,23 excluded for ethernet module
=======
uint8_t ValidPins0[] = { 0,2,4,13,14,15,16,17,21,22,25,26,27,32,33,34,35,36,39 };	// SPI pins 5,18,19,23 excluded for ethernet module
>>>>>>> master
```

**Replace with:**
```cpp
// valid pins for each processor
uint8_t ValidPins0[] = { 0,2,4,5,7,13,14,15,16,17,21,22,25,26,27,32,33,47 };	// SPI pins 5,18,19,23 excluded for ethernet module
```

**Reason:** Keep custom hardware pins (5, 7, 47) from ESP32RC_refactor25.

### Conflict 2: PWM Setup (Line 158-183)

**Current conflict:**
```cpp
	// pwm frequency change from default 5000 Hz to 490 Hz, required for some valves to work
<<<<<<< HEAD
		// DRV8870 IN1
		ledcSetup(i * 2, PWM_FREQ, PWM_BITS);
		ledcAttachPin(Sensor[i].IN1, i * 2);
		
		// DRV8870 IN2
		ledcSetup(i * 2 + 1, PWM_FREQ, PWM_BITS);
		ledcAttachPin(Sensor[i].IN2, i * 2 + 1);
=======
		ledcAttach(Sensor[i].IN1, PWM_FREQ, PWM_BITS);
		ledcWrite(Sensor[i].IN1, 0);

		ledcAttach(Sensor[i].IN2, PWM_FREQ, PWM_BITS);
		ledcWrite(Sensor[i].IN2, 0);

		if (Sensor[i].FlowPin == MDL.WheelSpeedPin) WheelMatch = true;
	}

	// wheel speed sensor
	if (MDL.WheelSpeedPin != NC && !WheelMatch)
	{
		pinMode(MDL.WheelSpeedPin, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(MDL.WheelSpeedPin), ISR_Speed, FALLING);
>>>>>>> master
	}
```

**Replace with:**
```cpp
	// pwm frequency change from default 5000 Hz to 490 Hz, required for some valves to work
		ledcAttach(Sensor[i].IN1, PWM_FREQ, PWM_BITS);
		ledcWrite(Sensor[i].IN1, 0);

		ledcAttach(Sensor[i].IN2, PWM_FREQ, PWM_BITS);
		ledcWrite(Sensor[i].IN2, 0);

		if (Sensor[i].FlowPin == MDL.WheelSpeedPin) WheelMatch = true;
	}

	// wheel speed sensor
	if (MDL.WheelSpeedPin != NC && !WheelMatch)
	{
		pinMode(MDL.WheelSpeedPin, INPUT_PULLUP);
		attachInterrupt(digitalPinToInterrupt(MDL.WheelSpeedPin), ISR_Speed, FALLING);
	}
```

After resolving both conflicts:
```bash
git add "Modules/ESP32 Rate/RC_ESP32/Begin.ino"
```

**Reason:** Adopt master's modern PWM API (ledcAttach/ledcWrite) and wheel speed sensor support.

---

## File 5: Modules/ESP32 Rate/RC_ESP32/PgNetwork.ino

**Conflict Location:** Line 9-99  
**Conflict:** CSS styling approach

### Resolution:

Find this conflict (it's about 90 lines of inline CSS):
```cpp
<<<<<<< HEAD
    st += GetCssStyle();
=======
    st += "    <style>";
    st += "      html { font-family: Helvetica, Arial, sans-serif; display:inline-block; margin:0 auto; text-align:center; }";
    // ... many lines of inline CSS ...
    st += "    </style>";
>>>>>>> master
```

Replace the entire conflict block with:
```cpp
    st += GetCssStyle();
```

Then stage the file:
```bash
git add "Modules/ESP32 Rate/RC_ESP32/PgNetwork.ino"
```

**Reason:** GetCssStyle() function exists in GUI.ino and provides cleaner code organization.

---

## Commit the Merge

After resolving all 5 conflicts and staging the files, commit:

```bash
git commit -m "Merge master into ESP32RC_refactor25

Resolved conflicts:
- Notes.txt: Kept ESP32RC_refactor25 version with flashing instructions
- RC_ESP32.ino.bin: Kept ESP32RC_refactor25 binary
- RC_ESP32.ino: Updated InoID to 30115 from master
- Begin.ino: Kept custom ValidPins array, adopted master's ledcAttach API
- PgNetwork.ino: Kept GetCssStyle() function approach

Integration preserves:
- Custom W5500 Ethernet support
- Custom I2C pins (SDA=8, SCL=18)
- Dual PCA9685 support
- Custom pin configurations
- ESP32RC_refactor25 hardware features

While incorporating:
- Master's updated InoID (30115)
- Master's newer PWM API (ledcAttach/ledcWrite)
- Master's wheel speed sensor support"
```

## Verify the Merge

```bash
# Check status (should show "nothing to commit")
git status

# View the merge commit
git log -1 --stat

# View file changes
git show HEAD
```

## Push to Remote

```bash
git push origin ESP32RC_refactor25
```

This will update PR #3 with the resolved merge.

---

## Quick Reference Commands

```bash
# Start
git checkout ESP32RC_refactor25
git merge master --no-commit --no-ff

# Resolve
git checkout --ours "Modules/ESP32 Rate/Notes.txt"
git checkout --ours "Modules/ESP32 Rate/RC_ESP32.ino.bin"
# Edit RC_ESP32.ino manually (InoID: 20115 → 30115)
# Edit Begin.ino manually (keep ValidPins, use new PWM API)
# Edit PgNetwork.ino manually (keep GetCssStyle())

# Stage all
git add "Modules/ESP32 Rate/Notes.txt"
git add "Modules/ESP32 Rate/RC_ESP32.ino.bin"
git add "Modules/ESP32 Rate/RC_ESP32/RC_ESP32.ino"
git add "Modules/ESP32 Rate/RC_ESP32/Begin.ino"
git add "Modules/ESP32 Rate/RC_ESP32/PgNetwork.ino"

# Commit
git commit -m "Merge master into ESP32RC_refactor25 [see message above]"

# Push
git push origin ESP32RC_refactor25
```
