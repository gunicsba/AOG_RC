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

	st += "\n  <tr><td>PID enabled</td><td>";
	st += PIDenabled[0];
	st += "  ";
	st += PIDenabled[1];
	st += "</td><td>0/1</td></tr>";

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

	// Current sense
	st += "\n  <tr><td>Current (Cytron)</td><td>";
	st += getCurrentInAmps(Current2Pin);
	st += " A</td><td>Motor current draw</td></tr>";

	st += "\n  <tr><td>Current (Sections)</td><td>";
	st += getCurrentInAmps(Current1Pin);
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
	st += "\n  </body></html>";

	return st;
}
