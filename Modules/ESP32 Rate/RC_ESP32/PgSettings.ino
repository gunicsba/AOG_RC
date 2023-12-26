
String GetPage2()
{
	String st = HtmlGetHead("Settings");
	st += "";
	st += "  <body>";
	st += "";
	st += "    <h1 align=center>Settings </h1>";
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
	st += "        <tr>";
	st += "          <td align='left'>Module Name</td>";
	st += "          <td><input class='InputCell' size='20' name='prop3' value='" + String(MDL.Name) + "' ID=Text3></td>";
	st += "        </tr>";
	st += "        <tr>";
	st += "          <td align='left'>ADS1115 address</td>";
	st += "          <td><input class='InputCell' size='20' name='prop4' value='" + String(MDL.AdsAddress) + "' ID=Text4></td>";
	st += "        </tr>";
	st += "      </table>";
	st += "";
	st += "      <p> <input class='button-72' id=Submit1 type=submit value='Save/Restart'></p>";
	st += "      <p> <a href='/page0'>Back</a> </p>";
	st += "    </form>";
	st += "";
  	st += "  </body>";
	st += "</html>";

	return st;
}






