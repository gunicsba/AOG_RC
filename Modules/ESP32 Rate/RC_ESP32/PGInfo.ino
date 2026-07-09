// PID runtime variables from PID.ino (compiled after PGInfo.ino)
extern float IntegralSum[];
extern float LastPWM[];

String GetPageInfo()
{
	String st = HtmlGetHead("Info");
	st += "\n    <h1>Info</h1>";
	st += "\n    <form id=FORM1 method=post action='/settings'>&nbsp;";

	// Temperature
	st += "\n<table border='1' style='border-collapse:collapse; margin:auto;'>";
	st += "\n  <tr><th>Parameter</th><th>Value</th><th>Description</th></tr>";

	st += "\n  <tr><td>ESP Temperature</td><td>";
	st += getChipTempC();
	st += " &deg;C</td><td>Internal chip temperature</td></tr>";

	st += "\n  <tr><td>Total Pulses S0</td><td>";
	st += Sensor[0].TotalPulses;
	st += "</td><td>Flow sensor pulse count</td></tr>";

	st += "\n  <tr><td>Total Pulses S1</td><td>";
	st += Sensor[1].TotalPulses;
	st += "</td><td>Flow sensor pulse count</td></tr>";

	// Module info
	st += "\n  <tr><td colspan='3'><b>Module Info</b></td></tr>";

	st += "\n  <tr><td>ID</td><td>";
	st += MDL.ID;
	st += "</td><td></td></tr>";

	st += "\n  <tr><td>SensorCount</td><td>";
	st += MDL.SensorCount;
	st += "</td><td></td></tr>";

	st += "\n  <tr><td>InvertRelay</td><td>";
	st += MDL.InvertRelay;
	st += "</td><td>Value that turns on relays</td></tr>";

	st += "\n  <tr><td>InvertFlow</td><td>";
	st += MDL.InvertFlow;
	st += "</td><td>Sets flow valve direction</td></tr>";

	st += "\n  <tr><td>ControlType</td><td>";
	st += Sensor[0].ControlType;
	st += "  ";
	st += Sensor[1].ControlType;
	st += "</td><td>0=standard, 1=combo, 2=motor, 4=fan, 5=timed</td></tr>";

	st += "\n  <tr><td>OnboardRelay</td><td>";
	st += MDL.OnboardRelayControl;
	st += "</td><td>5=PCA9685 (fixed)</td></tr>";

	st += "\n  <tr><td>Is3Wire</td><td>";
	st += MDL.Is3Wire ? "Yes (3-wire)" : "No (2-wire)";
	st += "</td><td>Valve wiring mode</td></tr>";

	st += "\n  <tr><td>FlowMasterValve</td><td>";
	if (FlowMasterValveIndex == 255)
		st += "Disabled";
	else
		st += FlowMasterValveIndex;
	st += "</td><td>0-15 or 255=disabled</td></tr>";

	st += "\n  <tr><td>Relay value</td><td>";
	st += RelayLo;
	st += "  ";
	st += RelayHi;
	st += "</td><td>RelayLo / RelayHi</td></tr>";

	st += "\n  <tr><td>Sensor Connected</td><td>";
	st += SensorConnected[0];
	st += "  ";
	st += SensorConnected[1];
	st += "</td><td>0/1</td></tr>";

	st += "\n  <tr><td>Applying</td><td>";
	st += Applying[0];
	st += "  ";
	st += Applying[1];
	st += "</td><td>0/1</td></tr>";
	
	st += "\n  <tr><td>PID Enabled</td><td>";
	st += PIDenabled[0];
	st += "  ";
	st += PIDenabled[1];
	st += "</td><td>0/1</td></tr>";
	
	st += "\n  <tr><td>MasterOn</td><td colspan=\"2\">";
	st += MasterOn;
	st += "</td><td>global from PGN32500 bit 4</td></tr>";
	
	st += "\n  <tr><td>AutoOn</td><td>";
	st += AutoOn[0];
	st += "  ";
	st += AutoOn[1];
	st += "</td><td>per-sensor from PGN32500 bit 6</td></tr>";
	
	st += "\n  <tr><td>SensorConnected</td><td>";
	st += SensorConnected[0];
	st += "  ";
	st += SensorConnected[1];
	st += "</td><td>CommTime within 4s</td></tr>";
	
	st += "\n  <tr><td>CalibrationOn</td><td>";
	st += CalibrationOn[0];
	st += "  ";
	st += CalibrationOn[1];
	st += "</td><td>PGN32500 bit 7 per sensor</td></tr>";
	
	st += "\n  <tr><td>CommTime delta (ms)</td><td>";
	st += (Sensor[0].CommTime > 0) ? (millis() - Sensor[0].CommTime) : (uint32_t)999999;
	st += "  ";
	st += (Sensor[1].CommTime > 0) ? (millis() - Sensor[1].CommTime) : (uint32_t)999999;
	st += "</td><td>ms since last PGN32500</td></tr>";

	// PWM values
	st += "\n  <tr><td>PWM S0</td><td>";
	st += Sensor[0].PWM;
	st += "</td><td>Current PWM value</td></tr>";

	st += "\n  <tr><td>PWM S1</td><td>";
	st += Sensor[1].PWM;
	st += "</td><td>Current PWM value</td></tr>";

	st += "\n  <tr><td>UPM S0</td><td>";
	st += Sensor[0].UPM;
	st += "</td><td>Measured flow rate</td></tr>";

	st += "\n  <tr><td>UPM S1</td><td>";
	st += Sensor[1].UPM;
	st += "</td><td>Measured flow rate</td></tr>";

	st += "\n  <tr><td>TargetUPM S0</td><td>";
	st += Sensor[0].TargetUPM;
	st += "</td><td>Requested flow rate</td></tr>";

	st += "\n  <tr><td>TargetUPM S1</td><td>";
	st += Sensor[1].TargetUPM;
	st += "</td><td>Requested flow rate</td></tr>";

	// Current sense (20A sensor, +0.5A calibration offset)
	st += "\n  <tr><td>Current (Cytron)</td><td>";
	st += (getCurrentInAmps(Current2Pin, 20.0f) + 0.3f);
	st += " A</td><td>Motor current draw</td></tr>";

	st += "\n  <tr><td>Current (Sections)</td><td>";
	st += (getCurrentInAmps(Current1Pin, 20.0f) + 0.3f);
	st += " A</td><td>Section relay current</td></tr>";

	// Ethernet status
	st += "\n  <tr><td>Ethernet</td><td>";
	st += ETHconnected ? "Connected" : "Disconnected";
	st += "</td><td></td></tr>";

	st += "\n  <tr><td>WiFi AP IP</td><td>";
	st += WiFi.softAPIP().toString();
	st += "</td><td></td></tr>";

	if (MDLnetwork.WifiModeUseStation)
	{
		st += "\n  <tr><td>WiFi Station IP</td><td>";
		st += WiFi.localIP().toString();
		st += "</td><td></td></tr>";
	}

	st += "\n</table>";

	// PID Debug table — S0 / S1 side by side
	st += "\n<br><h2>PID Debug</h2>";
	st += "\n<table border='1' style='border-collapse:collapse; margin:auto;'>";
	st += "\n  <tr><th>Parameter</th><th>S0</th><th>S1</th></tr>";

	// Control type
	static const char* ctNames[] = {"std", "combo", "motor", "?", "fan", "timed"};
	st += "\n  <tr><td>ControlType</td><td>";
	st += Sensor[0].ControlType;
	st += " (";
	st += ctNames[Sensor[0].ControlType <= 5 ? Sensor[0].ControlType : 3];
	st += ")</td><td>";
	st += Sensor[1].ControlType;
	st += " (";
	st += ctNames[Sensor[1].ControlType <= 5 ? Sensor[1].ControlType : 3];
	st += ")</td></tr>";

	// Hz (runtime)
	st += "\n  <tr><td>Hz</td><td>";
	st += Sensor[0].Hz;
	st += "</td><td>";
	st += Sensor[1].Hz;
	st += "</td></tr>";

	// RateError (runtime: TargetUPM - UPM)
	st += "\n  <tr><td>RateError</td><td>";
	st += (Sensor[0].TargetUPM - Sensor[0].UPM);
	st += "</td><td>";
	st += (Sensor[1].TargetUPM - Sensor[1].UPM);
	st += "</td></tr>";

	// IntegralSum (runtime)
	st += "\n  <tr><td>IntegralSum</td><td>";
	st += IntegralSum[0];
	st += "</td><td>";
	st += IntegralSum[1];
	st += "</td></tr>";

	// LastPWM (runtime)
	st += "\n  <tr><td>LastPWM</td><td>";
	st += LastPWM[0];
	st += "</td><td>";
	st += LastPWM[1];
	st += "</td></tr>";

	// Kp
	st += "\n  <tr><td>Kp</td><td>";
	st += Sensor[0].Kp;
	st += "</td><td>";
	st += Sensor[1].Kp;
	st += "</td></tr>";

	// Ki
	st += "\n  <tr><td>Ki</td><td>";
	st += Sensor[0].Ki;
	st += "</td><td>";
	st += Sensor[1].Ki;
	st += "</td></tr>";

	// Deadband
	st += "\n  <tr><td>Deadband</td><td>";
	st += Sensor[0].Deadband;
	st += "</td><td>";
	st += Sensor[1].Deadband;
	st += "</td></tr>";

	// BrakePoint
	st += "\n  <tr><td>BrakePoint</td><td>";
	st += Sensor[0].BrakePoint;
	st += "</td><td>";
	st += Sensor[1].BrakePoint;
	st += "</td></tr>";

	// PIDslowAdjust
	st += "\n  <tr><td>PIDslowAdjust</td><td>";
	st += Sensor[0].PIDslowAdjust;
	st += "</td><td>";
	st += Sensor[1].PIDslowAdjust;
	st += "</td></tr>";

	// SlewRate
	st += "\n  <tr><td>SlewRate</td><td>";
	st += Sensor[0].SlewRate;
	st += "</td><td>";
	st += Sensor[1].SlewRate;
	st += "</td></tr>";

	// MaxIntegral
	st += "\n  <tr><td>MaxIntegral</td><td>";
	st += Sensor[0].MaxIntegral;
	st += "</td><td>";
	st += Sensor[1].MaxIntegral;
	st += "</td></tr>";

	// MinPWM
	st += "\n  <tr><td>MinPWM</td><td>";
	st += Sensor[0].MinPWM;
	st += "</td><td>";
	st += Sensor[1].MinPWM;
	st += "</td></tr>";

	// MaxPWM
	st += "\n  <tr><td>MaxPWM</td><td>";
	st += Sensor[0].MaxPWM;
	st += "</td><td>";
	st += Sensor[1].MaxPWM;
	st += "</td></tr>";

	// PIDtime
	st += "\n  <tr><td>PIDtime</td><td>";
	st += Sensor[0].PIDtime;
	st += "</td><td>";
	st += Sensor[1].PIDtime;
	st += "</td></tr>";

	// TimedMinStart
	st += "\n  <tr><td>TimedMinStart</td><td>";
	st += Sensor[0].TimedMinStart;
	st += "</td><td>";
	st += Sensor[1].TimedMinStart;
	st += "</td></tr>";

	// TimedAdjust
	st += "\n  <tr><td>TimedAdjust</td><td>";
	st += Sensor[0].TimedAdjust;
	st += "</td><td>";
	st += Sensor[1].TimedAdjust;
	st += "</td></tr>";

	// TimedPause
	st += "\n  <tr><td>TimedPause</td><td>";
	st += Sensor[0].TimedPause;
	st += "</td><td>";
	st += Sensor[1].TimedPause;
	st += "</td></tr>";

	// PulseMin
	st += "\n  <tr><td>PulseMin</td><td>";
	st += Sensor[0].PulseMin;
	st += "</td><td>";
	st += Sensor[1].PulseMin;
	st += "</td></tr>";

	// PulseMax
	st += "\n  <tr><td>PulseMax</td><td>";
	st += Sensor[0].PulseMax;
	st += "</td><td>";
	st += Sensor[1].PulseMax;
	st += "</td></tr>";

	// MedianCount (live pulse sample count used in latest median)
	st += "\n  <tr><td>MedianCount</td><td>";
	st += MedianCount[0];
	st += "</td><td>";
	st += MedianCount[1];
	st += "</td></tr>";

	st += "\n</table>";

	// Feature flags section
	st += "\n<br><h2>Feature Flags</h2>";
	st += "\n<table border='1' style='border-collapse:collapse; margin:auto;'>";

	st += "\n<tr><td>Disable Motor (8th relay)</td>";
	st += "\n<td><input type='checkbox' name='disableMotor' ";
	st += disableMotor ? "checked" : "";
	st += "></td></tr>";

	st += "\n<tr><td>Disable Flow (8th relay)</td>";
	st += "\n<td><input type='checkbox' name='disableFlow' ";
	st += disableFlow ? "checked" : "";
	st += "></td></tr>";

	st += "\n<tr><td>9th relay controls S1 motor</td>";
	st += "\n<td><input type='checkbox' name='b9threlay' ";
	st += b9threlay ? "checked" : "";
	st += "></td></tr>";

	st += "\n</table>";

	st += "\n<br><input class='button-72' type=submit value='Save Settings'>";
	st += "\n      <p> <a class='button-72' href='/'>Back</a> </p>";
	st += "\n    </form>";

	// Auto-refresh: update data tables every second via fetch (skip Feature Flags table with checkboxes)
	st += "\n<script>";
	st += "\nvar tables = document.querySelectorAll('#FORM1 table');";
	st += "\nsetInterval(function() {";
	st += "\n  fetch('/info').then(function(r){return r.text()}).then(function(html){";
	st += "\n    var doc = new DOMParser().parseFromString(html,'text/html');";
	st += "\n    var t = doc.querySelectorAll('#FORM1 table');";
	st += "\n    var last = tables.length - 1;";
	st += "\n    for (var i = 0; i < last && i < t.length; i++)";
	st += "\n      tables[i].innerHTML = t[i].innerHTML;";
	st += "\n  });";
	st += "\n}, 1000);";
	st += "\n</script>";

	st += "\n  </body></html>";

	return st;
}
