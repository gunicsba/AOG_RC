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

void HandleSettings()
{
	bool changed = false;

	bool newState = server.arg("disableFlow") == "on";
	if (disableFlow != newState) { disableFlow = newState; changed = true; }

	newState = server.arg("disableMotor") == "on";
	if (disableMotor != newState) { disableMotor = newState; changed = true; }

	newState = server.arg("b9threlay") == "on";
	if (b9threlay != newState) { b9threlay = newState; changed = true; }

	if (changed) SaveData();

	HandleInfo();
}

void handleCredentials()
{
	bool OldMode = MDLnetwork.WifiModeUseStation;
	String OldSSID = String(MDLnetwork.SSID);
	String OldPassword = String(MDLnetwork.Password);
	String OldAPPassword = String(MDL.APpassword);

	String newSSID = server.arg("prop1");
	newSSID.trim();
	String newPassword = server.arg("prop2");
	newPassword.trim();

	// Hotspot/AP password (prop3). May be empty to make AP open.
	String newAPPassword = OldAPPassword;
	if (server.hasArg("prop3"))
	{
		newAPPassword = server.arg("prop3");
		newAPPassword.trim();

		// Enforce max length 
		const size_t kMaxApLen = 10;
		if (newAPPassword.length() > kMaxApLen)
		{
			newAPPassword.remove(kMaxApLen); 
		}
	}

	newSSID.toCharArray(MDLnetwork.SSID, sizeof(MDLnetwork.SSID));
	newPassword.toCharArray(MDLnetwork.Password, sizeof(MDLnetwork.Password));
	MDLnetwork.WifiModeUseStation = server.hasArg("connect");

	// Apply AP password if provided (including empty -> open network)
	if (server.hasArg("prop3"))
	{
		newAPPassword.toCharArray(MDL.APpassword, sizeof(MDL.APpassword));
	}

	server.send(200, "text/html", GetPage0());

	bool stationChanged =
		(MDLnetwork.WifiModeUseStation != OldMode) ||
		(String(MDLnetwork.SSID) != OldSSID) ||
		(String(MDLnetwork.Password) != OldPassword);

	bool apChanged = (String(MDL.APpassword) != OldAPPassword);

	if (stationChanged) SaveNetworks();
	if (apChanged) SaveData();

	if (stationChanged || apChanged)
	{
		delay(3000);
		ESP.restart();
	}
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

String HtmlGetHead(String title)
{
	String st = "";
	st += "<html>";
	st += "  <head>";
	st += "    <META content='text/html; charset=utf-8' http-equiv=Content-Type>";
	st += "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
	st += "    <title>";
	st += title;
	st += "</title>";
	st += "    <style>";
	st += "      html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }";
	st += "      h1 { color: #444444; margin: 50px auto 30px; }";
	st += "      body { margin-top: 50px; background-color: wheat; }";
	st += "      table.center { margin-left: auto; margin-right: auto; }";
	st += "      .button-72 {";
	st += "        align-items: center; background-color: initial;";
	st += "        background-image: linear-gradient(rgba(179,132,201,.84), rgba(57,31,91,.84) 50%);";
	st += "        border-radius: 42px; border-width: 0;";
	st += "        box-shadow: rgba(57,31,91,0.24) 0 2px 2px, rgba(179,132,201,0.4) 0 8px 12px;";
	st += "        color: #FFFFFF; cursor: pointer; display: inline-flex; justify-content: center; align-items: center;";
	st += "        font-family: Quicksand, sans-serif; font-size: 18px; font-weight: 700;";
	st += "        letter-spacing: .04em; line-height: 16px; margin: 12px auto; padding: 12px 18px;";
	st += "        text-align: center; text-decoration: none; user-select: none; -webkit-user-select: none;";
	st += "        touch-action: manipulation; vertical-align: baseline; width: 320px; max-width: 90%;";
	st += "      }";
	st += "      .button-on {";
	st += "        align-items: center; background-color: initial;";
	st += "        background-image: linear-gradient(rgba(50,255,50,.84), rgba(30,150,30,.84) 50%);";
	st += "        border-radius: 42px; border-width: 0;";
	st += "        box-shadow: rgba(57,31,91,0.24) 0 2px 2px, rgba(179,132,201,0.4) 0 8px 12px;";
	st += "        color: #FFFFFF; cursor: pointer; display: flex;";
	st += "        font-family: Quicksand, sans-serif; font-size: 18px; font-weight: 700;";
	st += "        justify-content: center; letter-spacing: .04em; line-height: 16px;";
	st += "        margin: auto; padding: 18px 18px; text-align: center; text-decoration: none;";
	st += "        user-select: none; -webkit-user-select: none; touch-action: manipulation;";
	st += "        vertical-align: baseline; width: 40%; max-width: 20em;";
	st += "      }";
	st += "      .button-off {";
	st += "        align-items: center; background-color: initial;";
	st += "        background-image: linear-gradient(rgba(255,50,50,.84), rgba(150,30,30,.84) 50%);";
	st += "        border-radius: 42px; border-width: 0;";
	st += "        box-shadow: rgba(57,31,91,0.24) 0 2px 2px, rgba(179,132,201,0.4) 0 8px 12px;";
	st += "        color: #FFFFFF; cursor: pointer; display: flex;";
	st += "        font-family: Quicksand, sans-serif; font-size: 18px; font-weight: 700;";
	st += "        justify-content: center; letter-spacing: .04em; line-height: 16px;";
	st += "        margin: auto; padding: 18px 18px; text-align: center; text-decoration: none;";
	st += "        user-select: none; -webkit-user-select: none; touch-action: manipulation;";
	st += "        vertical-align: baseline; width: 40%; max-width: 150px;";
	st += "      }";
	st += "      .InputCell { text-align: center; font-size: 18px; font-weight: 700; }";
	st += "      a:link { font-size: 150%; }";
	st += "    </style>";
	st += "  </head>";
	st += "  <body>";
	return st;
}

