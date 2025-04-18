
String GetPageInfo()
{
 	String st = HtmlGetHead("Info");
	st += "\n  <body>";
	st += "\n";
	st += "\n    <h1 align=center>Info</h1>";
	st += "\n    <form id=FORM1 method=post action='/Cytron'>&nbsp;";
	st += "\n";
	st += "\n";

  st += "\nKeresett a feri!";
  st += "\n<table>";
  st += "\n  <tr>";
  st += "\n    <th>Paraméter</th>";
  st += "\n    <th>Érték</th>";
  st += "\n    <th>Leírás</th>";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>Ciklusidő</td><td>";
  st += MaxLoopTime;
  st += "\n</td><td>uS -ban 10.000 az 0.01mp 2-5000 körüli érték normális. </td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>Leglassabb lépés</td><td>";
//  st += SlowestStep;
  st += "\n</td><td>A lépés ideje. </td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>Leglassabb lépés</td><td>";
  //st += SlowestStepName;
  st += "\n</td><td>A lépés neve. </td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>ESP maghőmérséklet</td><td>";
  float result = 0;
  temp_sensor_read_celsius(&result);
  st += result;
  st += "\n</td><td>Valószínűleg jó pár fokkal melegebb mint a környezet. 100 fölött aggódnék :) </td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>Összes pulzusszám szenzor 0</td><td>";
  st += Sensor[0].TotalPulses;
  st += "\n</td><td>Hasznos ha az átfolyásmérőt akarjuk ellenőrizni. </td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n<tr><td>Modulinfó</td></tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>ID</td><td>";
  st += MDL.ID;
  st += "\n</td><td></td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>SensorCount</td><td>";
  st += MDL.SensorCount;
  st += "\n</td><td></td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>RelayOnSignal</td><td>";
  st += MDL.RelayOnSignal;
  st += "\n</td><td>value that turns on relays (nincs használva)</td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>FlowOnDirection</td><td>";
  st += MDL.FlowOnDirection;
  st += "\n</td><td>sets on value for flow valve or sets motor direction (nincs használva)</td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";

  st += "\n<td>ControlType</td><td>";
  st += Sensor[0].ControlType;
  st += " ";
  st += Sensor[1].ControlType;
  st += "\n</td><td>// 0 standard, 1 combo close, 2 motor, 3 motor/weight, 4 fan, 5 timed combo </td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";


  st += "\n<td>Relékontroll</td><td>";
  st += MDL.RelayControl;
  st += "\n</td><td>0 - no relays, 1 - RS485, 2 - PCA9555 8 relays, 3 - PCA9555 16 relays, 4 - MCP23017, 5 - Teensy GPIO, 6 - PCA9685 </td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>Relay value</td><td>";
  st += RelayLo;
  st += "  ";
  st += RelayHi;
  st += "\n</td><td>RelayLo RelayHi</td> ";
  st += "\n  </tr>";
  st += "\n";
  st += "\n  <tr>";
  st += "\n<td>Flow enabled?</td><td>";
  st += Sensor[0].FlowEnabled;
  st += "   ";
  st += Sensor[1].FlowEnabled;
  st += "\n</td><td>0 / 1</td> ";
  st += "\n  </tr>";
  st += "\n";
/*
  st += "\n<tr><td>Cytronhoz :)</td></tr>";
  st += "\n";
  st += "\n<tr><td>PWM1</td>";
  st += "\n<td>";
  st += Sensor[1].PWM;
  st += "\n<input type='range' id='pwm1' name='pwm1' min='-255' max='255' value='";
  st += Sensor[1].PWM;
  st += "\n'/></td><td>húzd mínuszba a másik irányhoz :) 70 alatt ESP sleep!! </td>";
  st += "\n  </tr>";
*/

  st += "\n<tr><td>Amper Cytron: </td>";
  st += "\n";
  st += "\n<td>";
  st += getCurrentInAmps(Current2Pin);
  st += "\n  </td></tr>";

  st += "\n<tr><td>Amper szakasz: </td>";
  st += "\n";
  st += "\n<td>";
  st += getCurrentInAmps(Current1Pin);
  st += "\n  </td></tr>";


  st += "\n<tr><td>PID0</td>";
  st += "\n<td>Debug info</td>";
  st += "\n<td>";
  st += "\n\n";
  st += getDebugPID(0);
  st += "\n</td>";
  st += "\n</tr>";

  st += "\n<tr><td>Disable Motor drive based on 8th relay (Árpád)</td>";
  st += "\n<td></td>";
  st += "\n<td>";
  st += "\n\n";
  st += "<input type='checkbox' id='disableMotor' name='disableMotor' ";
  st += disableMotor ? "checked" : "";
  st += ">";
  st += "\n</td>";
  st += "\n</tr>";


  st += "\n<tr><td>Disable Flow value based on 8th relay (Baranyi)</td>";
  st += "\n<td></td>";
  st += "\n<td>";
  st += "\n\n";
  st += "<input type='checkbox' id='disableFlow' name='disableFlow' ";
  st += disableFlow ? "checked" : "";
  st += ">";
  st += "\n</td>";
  st += "\n</tr>";

  st += "\n</table>";
  st += "\n";
  st += "\n<input class='bar' name='foo' type=submit formaction='/Cytron' value='Frissíts!'>";
  st += "\n      <p> <a href='/page0'>Back</a> </p>";
	st += "\n    </form>";
  st += "\n  </body>";
	st += "\n";
	st += "\n</html>";
//  SlowestStep = 0;
	return st;
}

