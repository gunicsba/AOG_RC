void HandleRoot()
{
	if (server.hasArg("prop1"))
	{
		handleCredentials();
	}
	else
	{
		server.send(200, "text/html", GetPage0());
	}
}

void HandlePage1()
{
	// switches
	server.send(200, "text/html", GetPage1());
}

void HandlePage2()
{
	// network
	server.send(200, "text/html", GetPage2());
}

void HandleInfo()
{
  server.send(200, "text/html", GetPageInfo());
}
void handleCredentials()
{
	int NewID;
	int Interval;

	server.arg("prop1").toCharArray(MDL.SSID, sizeof(MDL.SSID) - 1);
	server.arg("prop2").toCharArray(MDL.Password, sizeof(MDL.Password) - 1);
	MDL.WifiMode = 1;

	server.send(200, "text/html", GetPage0());

	SaveData();

	delay(3000);

	ESP.restart();
}

void Cytron() {

  bool newState = server.arg("disableFlow") == "on";
  if(disableFlow != newState) {
    disableFlow = newState;
    SaveData();
  }
  newState = server.arg("disableMotor") == "on";
  if(disableMotor != newState) {
    disableMotor = newState;
    SaveData();
  }
  
  HandleInfo();

}

void ButtonPressed()
{
	if (server.arg("Btn") == "Master")
	{
		WifiMasterOn = !WifiMasterOn;
		WifiSwitchesTimer = millis();
		HandlePage1();
	}
	else
	{
		int ID = server.arg("Btn").toInt() - 1;
		if (ID >= 0 && ID < 16)
		{
			Button[ID] = !Button[ID];
			WifiSwitchesTimer = millis();
			HandlePage1();
		}
	}
}

String HtmlGetHead(String title) {
  String st="";
 	st += "<html>";
  st += "  <head>";
	st += "    <META content='text/html; charset=utf-8' http-equiv=Content-Type>";
	st += "    <meta name=vs_targetSchema content='HTML 4.0'>";
	st += "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
	st += "    <title>";
  st += title;
  st += "</title>";
  st += "    <style>";
	st += "      html {";
	st += "        font-family: Helvetica;";
	st += "        display: inline-block;";
	st += "        margin: 0px auto;";
	st += "        text-align: center;";
	st += "";
	st += "      }";
	st += "";
	st += "      h1 {";
	st += "        color: #444444;";
	st += "        margin: 50px auto 30px;";
	st += "      }";
	st += "";
	st += "      table.center {";
	st += "        margin-left: auto;";
	st += "        margin-right: auto;";
	st += "      }";
	st += "";
	st += "      .button-72 {";
	st += "        align-items: center;";
	st += "        background-color: initial;";
	st += "        background-image: linear-gradient(rgba(179, 132, 201, .84), rgba(57, 31, 91, .84) 50%);";
	st += "        border-radius: 42px;";
	st += "        border-width: 0;";
	st += "        box-shadow: rgba(57, 31, 91, 0.24) 0 2px 2px, rgba(179, 132, 201, 0.4) 0 8px 12px;";
	st += "        color: #FFFFFF;";
	st += "        cursor: pointer;";
	st += "        display: flex;";
	st += "        font-family: Quicksand, sans-serif;";
	st += "        font-size: 18px;";
	st += "        font-weight: 700;";
	st += "        justify-content: center;";
	st += "        letter-spacing: .04em;";
	st += "        line-height: 16px;";
	st += "        margin: auto;";
	st += "        padding: 18px 18px;";
	st += "        text-align: center;";
	st += "        text-decoration: none;";
	st += "        text-shadow: rgba(255, 255, 255, 0.4) 0 0 4px, rgba(255, 255, 255, 0.2) 0 0 12px, rgba(57, 31, 91, 0.6) 1px 1px 4px, rgba(57, 31, 91, 0.32) 4px 4px 16px;";
	st += "        user-select: none;";
	st += "        -webkit-user-select: none;";
	st += "        touch-action: manipulation;";
	st += "        vertical-align: baseline;";
	st += "        width: 40%";
	//st += "        max-width: 20em";
	st += "        max-width: 150px";
	st += "      }";
	st += "";
	st += "      .button-on {";
	st += "        align-items: center;";
	st += "        background-color: initial;";
	st += "        background-image: linear-gradient(rgba(50, 255, 50, .84), rgba(30, 150, 30, .84) 50%);";
	st += "        border-radius: 42px;";
	st += "        border-width: 0;";
	st += "        box-shadow: rgba(57, 31, 91, 0.24) 0 2px 2px, rgba(179, 132, 201, 0.4) 0 8px 12px;";
	st += "        color: #FFFFFF;";
	st += "        cursor: pointer;";
	st += "        display: flex;";
	st += "        font-family: Quicksand, sans-serif;";
	st += "        font-size: 18px;";
	st += "        font-weight: 700;";
	st += "        justify-content: center;";
	st += "        letter-spacing: .04em;";
	st += "        line-height: 16px;";
	st += "        margin: auto;";
	st += "        padding: 18px 18px;";
	st += "        text-align: center;";
	st += "        text-decoration: none;";
	st += "        text-shadow: rgba(255, 255, 255, 0.4) 0 0 4px, rgba(255, 255, 255, 0.2) 0 0 12px, rgba(57, 31, 91, 0.6) 1px 1px 4px, rgba(57, 31, 91, 0.32) 4px 4px 16px;";
	st += "        user-select: none;";
	st += "        -webkit-user-select: none;";
	st += "        touch-action: manipulation;";
	st += "        vertical-align: baseline;";
	st += "        width: 40%";
	st += "        max-width: 20em";
	st += "      }";
	st += "";
	st += "      .button-off {";
	st += "        align-items: center;";
	st += "        background-color: initial;";
	st += "        background-image: linear-gradient(rgba(255, 50, 50, .84), rgba(150, 30, 30, .84) 50%);";
	st += "        border-radius: 42px;";
	st += "        border-width: 0;";
	st += "        box-shadow: rgba(57, 31, 91, 0.24) 0 2px 2px, rgba(179, 132, 201, 0.4) 0 8px 12px;";
	st += "        color: #FFFFFF;";
	st += "        cursor: pointer;";
	st += "        display: flex;";
	st += "        font-family: Quicksand, sans-serif;";
	st += "        font-size: 18px;";
	st += "        font-weight: 700;";
	st += "        justify-content: center;";
	st += "        letter-spacing: .04em;";
	st += "        line-height: 16px;";
	st += "        margin: auto;";
	st += "        padding: 18px 18px;";
	st += "        text-align: center;";
	st += "        text-decoration: none;";
	st += "        text-shadow: rgba(255, 255, 255, 0.4) 0 0 4px, rgba(255, 255, 255, 0.2) 0 0 12px, rgba(57, 31, 91, 0.6) 1px 1px 4px, rgba(57, 31, 91, 0.32) 4px 4px 16px;";
	st += "        user-select: none;";
	st += "        -webkit-user-select: none;";
	st += "        touch-action: manipulation;";
	st += "        vertical-align: baseline;";
	st += "        width: 40%";
	//st += "        max-width: 20em";
	st += "        max-width: 150px";
	st += "      }";
  st += "";
	st += "      .InputCell {";
	st += "        text-align: center;";
	st += "        font-size: 18px;";
	st += "        font-weight: 700;";
	st += "      }";
	st += "";
	st += "      a:link {";
	st += "        font-size: 150%;";
	st += "      }";
	st += "";
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

  return st;
}


