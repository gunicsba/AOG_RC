
String tmp;
String GetPage1()
{
	String st = HtmlGetHead("Switches");
	st += "    <h1>Switches</h1>";
	st += "    <form id=FORM1 method=post action='/'>&nbsp;";

	if (WifiMasterOn) tmp = "button-on"; else tmp = "button-off";
	st += "      <p> <input class='" + tmp + "' name='Btn' type=submit formaction='/ButtonPressed' value='Master'> </p>";

	for (int i = 0; i < 16; i++)
	{
		if (Button[i]) tmp = "button-on"; else tmp = "button-off";
		st += "      <p> <input class='" + tmp + "' name='Btn' type=submit formaction='/ButtonPressed' value='" + String(i + 1) + "'> </p>";
	}

	st += "      <p> <a href='/'>Back</a> </p>";
	st += "    </form>";
	st += "</body></html>";

	return st;
}

