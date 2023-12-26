
String GetPageInfo()
{
 	String st = HtmlGetHead("Info");
	st += "  <body>";
	st += "";
	st += "    <h1 align=center>Info</h1>";
	st += "    <form id=FORM1 method=post action='/Cytron'>&nbsp;";
	st += "";
	st += "";

  st += "Keresett a feri!";
  st += "<table>";
  st += "  <tr>";
  st += "    <th>Paraméter</th>";
  st += "    <th>Érték</th>";
  st += "    <th>Leírás</th>";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>Ciklusidő</td><td>";
  st += MaxLoopTime;
  st += "</td><td>uS -ban 10.000 az 0.01mp 2-5000 körüli érték normális. </td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>Leglassabb lépés</td><td>";
  st += SlowestStep;
  st += "</td><td>A lépés ideje. </td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>Leglassabb lépés</td><td>";
  st += SlowestStepName;
  st += "</td><td>A lépés neve. </td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>ESP maghőmérséklet</td><td>";
  float result = 0;
  temp_sensor_read_celsius(&result);
  st += result;
  st += "</td><td>Valószínűleg jó pár fokkal melegebb mint a környezet. 100 fölött aggódnék :) </td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>Összes pulzusszám szenzor 0</td><td>";
  st += Sensor[0].TotalPulses;
  st += "</td><td>Hasznos ha az átfolyásmérőt akarjuk ellenőrizni. </td> ";
  st += "  </tr>";
  st += "";
  st += "<tr><td>Modulinfó</td></tr>";
  st += "";
  st += "  <tr>";
  st += "<td>ID</td><td>";
  st += MDL.ID;
  st += "</td><td></td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>SensorCount</td><td>";
  st += MDL.SensorCount;
  st += "</td><td></td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>RelayOnSignal</td><td>";
  st += MDL.RelayOnSignal;
  st += "</td><td>value that turns on relays (nincs használva)</td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>FlowOnDirection</td><td>";
  st += MDL.FlowOnDirection;
  st += "</td><td>sets on value for flow valve or sets motor direction (nincs használva)</td> ";
  st += "  </tr>";
  st += "";
  st += "  <tr>";
  st += "<td>Relékontroll</td><td>";
  st += MDL.RelayControl;
  st += "</td><td>0 - no relays, 1 - RS485, 2 - PCA9555 8 relays, 3 - PCA9555 16 relays, 4 - MCP23017, 5 - Teensy GPIO, 6 - PCA9685 </td> ";
  st += "  </tr>";
  st += "";
  st += "<tr><td>Cytronhoz :)</td></tr>";
  st += "";
  st += "<tr><td>PWM1</td>";
  st += "<td>";
  st += Sensor[1].PWM;
  st += "<input type='range' id='pwm1' name='pwm1' min='-255' max='255' value='";
  st += Sensor[1].PWM;
  st += "'/></td><td>húzd mínuszba a másik irányhoz :)</td>";
  st += "  </tr>";
  st += "</table>";
  st += "";
 	st += "      <p> <a href='/page0'>Back</a> </p>";
  st += "<input class='bar' name='foo' type=submit formaction='/Cytron' value='Master'>";
	st += "    </form>";
  st += "  </body>";
	st += "";
	st += "</html>";
  SlowestStep = 0;
	return st;
}





