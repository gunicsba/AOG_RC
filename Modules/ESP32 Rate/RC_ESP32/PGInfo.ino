// PID runtime variables from PID.ino (compiled after PGInfo.ino)
extern float IntegralSum[];
extern float LastPWM[];

// helper: emit one sensor <td> cell
static void SensorCell(String& st, const String& val)
{
	st += "<td>";
	st += val;
	st += "</td>";
}

// helper: emit N sensor cells by looping SensorCount
static void SensorCells(String& st, int count, const String vals[])
{
	for (int i = 0; i < count; i++)
		SensorCell(st, vals[i]);
}

// helper: emit a section header row spanning all columns
static void SectionRow(String& st, int cols, const char* title)
{
	st += "\n  <tr><td colspan='";
	st += cols;
	st += "'><b>";
	st += title;
	st += "</b></td></tr>";
}

String GetPageInfo()
{
	String st = HtmlGetHead("Info");
	st += "\n    <h1>Info</h1>";
	st += "\n    <form id=FORM1 method=post action='/settings'>&nbsp;";

	const int sc = MDL.SensorCount;   // shorthand
	const int sysCols = 3;             // Parameter | value | Description

	// ──────────────────────────────────────────────
	//  System table  (module-level, no per-sensor columns)
	// ──────────────────────────────────────────────
	st += "\n<table border='1' style='border-collapse:collapse; margin:auto;'>";
	st += "\n  <tr><th>Parameter</th><th>Value</th><th>Description</th></tr>";

	st += "\n  <tr><td>ESP Temperature</td><td>";
	st += getChipTempC();
	st += " &deg;C</td><td>Internal chip temperature</td></tr>";

	st += "\n  <tr><td>ID</td><td>";
	st += MDL.ID;
	st += "</td><td></td></tr>";

	st += "\n  <tr><td>SensorCount</td><td>";
	st += sc;
	st += "</td><td></td></tr>";

	st += "\n  <tr><td>InvertRelay</td><td>";
	st += MDL.InvertRelay;
	st += "</td><td>Value that turns on relays</td></tr>";

	st += "\n  <tr><td>InvertFlow</td><td>";
	st += MDL.InvertFlow;
	st += "</td><td>Sets flow valve direction</td></tr>";

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
	st += " / ";
	st += RelayHi;
	st += "</td><td>RelayLo / RelayHi</td></tr>";

	st += "\n  <tr><td>MasterOn</td><td>";
	st += MasterOn;
	st += "</td><td>global from PGN32500 bit 4</td></tr>";

	// Current sense
	st += "\n  <tr><td>Current (Cytron)</td><td>";
	st += (getCurrentInAmps(Current2Pin, 20.0f) + 0.3f);
	st += " A</td><td>Motor current draw</td></tr>";

	st += "\n  <tr><td>Current (Sections)</td><td>";
	st += (getCurrentInAmps(Current1Pin, 20.0f) + 0.3f);
	st += " A</td><td>Section relay current</td></tr>";

	// Ethernet / WiFi
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

	// ──────────────────────────────────────────────
	//  Sensor table  (all per-sensor data, scales with SensorCount)
	// ──────────────────────────────────────────────
	const int senCols = sc + 1;  // Parameter | S0 | S1 | ...

	st += "\n<br><h2>Sensors</h2>";
	st += "\n<table border='1' style='border-collapse:collapse; margin:auto;'>";
	st += "\n  <tr><th>Parameter</th>";
	for (int i = 0; i < sc; i++)
	{
		st += "<th>S";
		st += i;
		st += "</th>";
	}
	st += "</tr>";

	// ── Pins ──
	SectionRow(st, senCols, "Pins");

	// FlowPin
	st += "\n  <tr><td>FlowPin</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String((Sensor[i].FlowPin < NC) ? (int)Sensor[i].FlowPin : -1));
	st += "</tr>";

	// IN1
	st += "\n  <tr><td>IN1</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String((Sensor[i].IN1 < NC) ? (int)Sensor[i].IN1 : -1));
	st += "</tr>";

	// IN2
	st += "\n  <tr><td>IN2</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String((Sensor[i].IN2 < NC) ? (int)Sensor[i].IN2 : -1));
	st += "</tr>";

	// BinPin
	st += "\n  <tr><td>BinPin</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String((Sensor[i].BinPin < NC) ? (int)Sensor[i].BinPin : -1));
	st += "</tr>";

	// ── Status ──
	SectionRow(st, senCols, "Status");

	// ControlType
	static const char* ctNames[] = {"std", "combo", "motor", "?", "fan", "timed"};
	st += "\n  <tr><td>ControlType</td>";
	for (int i = 0; i < sc; i++)
	{
		byte ct = Sensor[i].ControlType;
		SensorCell(st, String(ct) + " (" + ctNames[ct <= 5 ? ct : 3] + ")");
	}
	st += "</tr>";

	// SensorConnected
	st += "\n  <tr><td>Connected</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(SensorConnected[i]));
	st += "</tr>";

	// Applying
	st += "\n  <tr><td>Applying</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Applying[i]));
	st += "</tr>";

	// PID Enabled
	st += "\n  <tr><td>PID Enabled</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(PIDenabled[i]));
	st += "</tr>";

	// AutoOn
	st += "\n  <tr><td>AutoOn</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(AutoOn[i]));
	st += "</tr>";

	// CalibrationOn
	st += "\n  <tr><td>CalibrationOn</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(CalibrationOn[i]));
	st += "</tr>";

	// CommTime delta
	st += "\n  <tr><td>CommTime delta (ms)</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String((Sensor[i].CommTime > 0) ? (millis() - Sensor[i].CommTime) : (uint32_t)999999));
	st += "</tr>";

	// TotalPulses
	st += "\n  <tr><td>TotalPulses</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].TotalPulses));
	st += "</tr>";

	// BinEmpty (live)
	st += "\n  <tr><td>BinEmpty</td>";
	for (int i = 0; i < sc; i++)
	{
		if (Sensor[i].BinPin >= NC)
			SensorCell(st, "N/A");
		else
			SensorCell(st, BinEmpty[i] ? "Empty" : "Full");
	}
	st += "</tr>";

	// ── Flow ──
	SectionRow(st, senCols, "Flow");

	// UPM
	st += "\n  <tr><td>UPM</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].UPM));
	st += "</tr>";

	// TargetUPM
	st += "\n  <tr><td>TargetUPM</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].TargetUPM));
	st += "</tr>";

	// RateError
	st += "\n  <tr><td>RateError</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].TargetUPM - Sensor[i].UPM));
	st += "</tr>";

	// Hz
	st += "\n  <tr><td>Hz</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].Hz));
	st += "</tr>";

	// MedianCount
	st += "\n  <tr><td>MedianCount</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(MedianCount[i]));
	st += "</tr>";

	// PulseMin / PulseMax
	st += "\n  <tr><td>PulseMin</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].PulseMin));
	st += "</tr>";

	st += "\n  <tr><td>PulseMax</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].PulseMax));
	st += "</tr>";

	// ── PID ──
	SectionRow(st, senCols, "PID");

	// PWM
	st += "\n  <tr><td>PWM</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].PWM));
	st += "</tr>";

	// LastPWM
	st += "\n  <tr><td>LastPWM</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(LastPWM[i]));
	st += "</tr>";

	// IntegralSum
	st += "\n  <tr><td>IntegralSum</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(IntegralSum[i]));
	st += "</tr>";

	// Kp
	st += "\n  <tr><td>Kp</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].Kp));
	st += "</tr>";

	// Ki
	st += "\n  <tr><td>Ki</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].Ki));
	st += "</tr>";

	// Deadband
	st += "\n  <tr><td>Deadband</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].Deadband));
	st += "</tr>";

	// BrakePoint
	st += "\n  <tr><td>BrakePoint</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].BrakePoint));
	st += "</tr>";

	// PIDslowAdjust
	st += "\n  <tr><td>PIDslowAdjust</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].PIDslowAdjust));
	st += "</tr>";

	// SlewRate
	st += "\n  <tr><td>SlewRate</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].SlewRate));
	st += "</tr>";

	// MaxIntegral
	st += "\n  <tr><td>MaxIntegral</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].MaxIntegral));
	st += "</tr>";

	// MinPWM
	st += "\n  <tr><td>MinPWM</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].MinPWM));
	st += "</tr>";

	// MaxPWM
	st += "\n  <tr><td>MaxPWM</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].MaxPWM));
	st += "</tr>";

	// PIDtime
	st += "\n  <tr><td>PIDtime</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].PIDtime));
	st += "</tr>";

	// TimedMinStart
	st += "\n  <tr><td>TimedMinStart</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].TimedMinStart));
	st += "</tr>";

	// TimedAdjust
	st += "\n  <tr><td>TimedAdjust</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].TimedAdjust));
	st += "</tr>";

	// TimedPause
	st += "\n  <tr><td>TimedPause</td>";
	for (int i = 0; i < sc; i++)
		SensorCell(st, String(Sensor[i].TimedPause));
	st += "</tr>";

	st += "\n</table>";

	st += "\n<br><input class='button-72' type=submit value='Save Settings'>";
	st += "\n      <p> <a class='button-72' href='/'>Back</a> </p>";
	st += "\n    </form>";

	// Auto-refresh: update data tables every second via fetch
	st += "\n<script>";
	st += "\nvar tables = document.querySelectorAll('#FORM1 table');";
	st += "\nsetInterval(function() {";
	st += "\n  fetch('/info').then(function(r){return r.text()}).then(function(html){";
	st += "\n    var doc = new DOMParser().parseFromString(html,'text/html');";
	st += "\n    var t = doc.querySelectorAll('#FORM1 table');";
	st += "\n    for (var i = 0; i < tables.length && i < t.length; i++)";
	st += "\n      tables[i].innerHTML = t[i].innerHTML;";
	st += "\n  });";
	st += "\n}, 1000);";
	st += "\n</script>";

	st += "\n  </body></html>";

	return st;
}
