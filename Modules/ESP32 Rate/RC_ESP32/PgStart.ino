
String GetPage0()
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
	st += "    <h1 align=center>RC_ESP32";
	st += "    </h1>";
	st += "    <form id=FORM1 method=post action='/'>&nbsp;";
	st += "";
	st += "";
	st += "      <p> <a class='button-72' href='/page1' >Switches</a> </p>";
	st += "      <p> <a class='button-72' href='/page2' >Network</a> </p>";
	st += "      <p> <a class='button-72' href='/update' >Update Firmware</a> </p>";
	st += "";
	st += "    </form>";
	st += "";
	st += "</HTML>";
  Serial.println(st);
	return st;
}

