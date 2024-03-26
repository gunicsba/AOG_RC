
String GetPage2()
{
  String st = HtmlGetHead("Wifi AOG");
	st += "";
	st += "  <BODY>";
	st += "    <style>";
	st += "      body {";
	st += "        margin-top: 50px;";
	st += "        background-color: wheat";
	st += "      }";
	st += "";
	st += "      font-family: Arial,";
	st += "      Helvetica,";
	st += "      Sans-Serif;";
	st += "";
	st += "    </style>";
	st += "";
	st += "    <h1 align=center>Wifi Network </h1>";
	st += "    <form id=FORM1 method=post action='/'>&nbsp;";
	st += "      <table class='center'>";
	st += "";
	st += "        <tr>";
	st += "          <td align='left'>Network</td>";
	st += "          <td><input class='InputCell' size='20' name='prop1' value='" + String(MDL.SSID) + "' ID=Text1></td>";
	st += "        </tr>";
	st += "";
	st += "        <tr>";
	st += "          <td align='left'>Password</td>";
	st += "          <td><input class='InputCell' size='20' name='prop2' value='" + String(MDL.Password) + "' ID=Text2></td>";
	st += "        </tr>";
	st += "      </table>";
	st += "";
	st += "      <p> <input class='button-72' id=Submit1 type=submit value='Save/Restart'></p>";
	st += "      <p> <a href='/page0'>Back</a> </p>";
	st += "    </form>";
	if (WiFi.isConnected())
	{
		st += "<p>Wifi Connected to " + String(MDL.SSID) + "</p>";
	}
	else
	{
		st += "<p>Wifi Not Connected</p>";
	}
	//st += "<P>Debug</p>";
	//st += "<p>" + String(debug1) + "</p>";
	//st += "<p>" + String(debug2) + "</p>";
	//st += "<p>" + String(debug3) + "</p>";
	st += "</body>";
	st += "";
	st += "</HTML>";
Serial.println(st);
	return st;
}






