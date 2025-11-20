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
		// zero-pad month/day for nicer display
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
	
	String st = "<HTML>";
	st += "";
	st += "  <head>";
	st += "    <META content='text/html; charset=utf-8' http-equiv=Content-Type>";
	st += "    <meta name=vs_targetSchema content='HTML 4.0'>";
	st += "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
	st += "    <title>Rate Control</title>";
  st += GetCssStyle();
	st += "  </head>";
	st += "";
	st += "  <BODY>";
	st += "    <style>";
	st += "      body {";
	st += "        margin-top: 50px;";
	st += "        background-color: wheat;";
	st += "      }";
	st += "";
	st += "      font-family: Arial,";
	st += "      Helvetica,";
	st += "      Sans-Serif;";
	st += "";
	st += "    </style>";
	st += "";
	st += "    <h1 align=center>RC_ESP32";
	st += "    </h1>";
	st += "    <p style='margin:0 0 12px 0; color:#666; font-size:14px;'>";
	st += fwVer;
	st += "</p>";
	st += "    <form id=FORM1 method=post action='/'>&nbsp;";
	st += "";
	st += "      <p> <a class='btn-base btn-purple' href='/page1' >Switches</a> </p>";
	st += "      <p> <a class='btn-base btn-purple' href='/page2' >Network</a> </p>";
	st += "      <p> <a class='btn-base btn-purple' href='/update' >Update Firmware</a> </p>";
	st += "";
	st += "    </form>";
	st += "";
	st += "</HTML>";

	return st;
}
