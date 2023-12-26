
String GetPage0()
{
	String st = HtmlGetHead("ESP32 Rate Module");
	st += "  <body>";
	st += "";
	st += "    <h1 align=center>ESP32 Rate Module";
	st += "    </h1>";
	st += "    <form id=FORM1 method=post action='/'>&nbsp;";
	st += "";
	st += "";
	st += "      <p> <a class='button-72' href='/page1' >Switches</a> </p>";
	st += "      <p> <a class='button-72' href='/page2' >Settings</a> </p>";
  st += "      <p> <a class='button-72' href='/info' >Info</a> </p>";
	st += "";
	st += "    </form>";
	st += "";
 	st += "  </body>";
  st += "";
	st += "</html>";

	return st;
}
