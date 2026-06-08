String GetPage0()
{
	// version
	uint16_t yr = InoID % 10 + 2020;
	uint16_t rest = InoID / 10;
	uint8_t mn = rest % 100;
	uint16_t dy = rest / 100;

	String fwVer;
	if (mn <= 12 && dy <= 31)
	{
		fwVer = "Firmware Version: v";
		fwVer += String(yr);
		fwVer += ".";
		if (mn < 10) fwVer += "0";
		fwVer += String(mn);
		fwVer += ".";
		if (dy < 10) fwVer += "0";
		fwVer += String(dy);
	}
	else
	{
		fwVer = "Firmware Version: invalid";
	}

	String st = HtmlGetHead("Rate Control");
	st += "    <h1>RC_ESP32</h1>";
	st += "    <p style='margin:0 0 12px 0; color:#666; font-size:14px;'>";
	st += fwVer;
	st += "</p>";
	st += "    <form id=FORM1 method=post action='/'>&nbsp;";
	st += "      <p> <a class='button-72' href='/page1' >Switches</a> </p>";
	st += "      <p> <a class='button-72' href='/page2' >Network</a> </p>";
	st += "      <p> <a class='button-72' href='/info' >Info</a> </p>";
	st += "      <p> <a class='button-72' href='/update' >Update Firmware</a> </p>";
	st += "    </form>";
	st += "</body></html>";

	return st;
}
