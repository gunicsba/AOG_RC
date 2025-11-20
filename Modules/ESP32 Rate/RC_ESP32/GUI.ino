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

void handleCredentials()
{
	bool OldMode = MDLnetwork.WifiModeUseStation;
	String OldSSID = String(MDLnetwork.SSID);
	String OldPassword = String(MDLnetwork.Password);

	String newSSID = server.arg("prop1");
	newSSID.trim();  
	String newPassword = server.arg("prop2");
	newPassword.trim();

	newSSID.toCharArray(MDLnetwork.SSID, sizeof(MDLnetwork.SSID));
	newPassword.toCharArray(MDLnetwork.Password, sizeof(MDLnetwork.Password));
	MDLnetwork.WifiModeUseStation = server.hasArg("connect");

	server.send(200, "text/html", GetPage0());

	if (MDLnetwork.WifiModeUseStation != OldMode ||
		String(MDLnetwork.SSID) != OldSSID ||
		String(MDLnetwork.Password) != OldPassword)
	{
		SaveNetworks();
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

// Handler for info page
void HandleInfo()
{
	server.send(200, "text/html", GetPageInfo());
}

// Handler for Cytron checkbox form
void Cytron()
{
	bool newState = server.arg("disableFlow") == "on";
	if (disableFlow != newState)
	{
		disableFlow = newState;
		SaveData();
	}
	newState = server.arg("disableMotor") == "on";
	if (disableMotor != newState)
	{
		disableMotor = newState;
		SaveData();
	}
	newState = server.arg("b9threlay") == "on";
	if (b9threlay != newState)
	{
		b9threlay = newState;
		SaveData();
	}
	
	HandleInfo();
}

// Helper function to generate consistent HTML head section
String HtmlGetHead(String title)
{
	String st = "";
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
	st += "      }";
	st += "      h1 {";
	st += "        color: #444444;";
	st += "        margin: 50px auto 30px;";
	st += "      }";
	st += "      h3 {";
	st += "        color: #444444;";
	st += "        margin-bottom: 50px;";
	st += "      }";
	st += "      p {";
	st += "        font-size: 20px;";
	st += "        color: #444444;";
	st += "        margin-bottom: 10px;";
	st += "      }";
	st += "      table {";
	st += "        border-collapse: collapse;";
	st += "        margin: 25px auto;";
	st += "        font-size: 18px;";
	st += "        min-width: 400px;";
	st += "        box-shadow: 0 0 20px rgba(0, 0, 0, 0.15);";
	st += "      }";
	st += "      thead tr {";
	st += "        background-color: #4CAF50;";
	st += "        color: #ffffff;";
	st += "        text-align: left;";
	st += "      }";
	st += "      th, td {";
	st += "        padding: 12px 15px;";
	st += "      }";
	st += "      tbody tr {";
	st += "        border-bottom: 1px solid #dddddd;";
	st += "      }";
	st += "      tbody tr:nth-of-type(even) {";
	st += "        background-color: #f3f3f3;";
	st += "      }";
	st += "      tbody tr:last-of-type {";
	st += "        border-bottom: 2px solid #4CAF50;";
	st += "      }";
	st += "      .button {";
	st += "        display: inline-block;";
	st += "        padding: 15px 25px;";
	st += "        font-size: 20px;";
	st += "        cursor: pointer;";
	st += "        text-align: center;";
	st += "        text-decoration: none;";
	st += "        outline: none;";
	st += "        color: #fff;";
	st += "        background-color: #4CAF50;";
	st += "        border: none;";
	st += "        border-radius: 15px;";
	st += "        box-shadow: 0 5px #999;";
	st += "        margin: 5px;";
	st += "      }";
	st += "      .button:hover {background-color: #3e8e41}";
	st += "      .button:active {";
	st += "        background-color: #3e8e41;";
	st += "        box-shadow: 0 2px #666;";
	st += "        transform: translateY(4px);";
	st += "      }";
	st += "      .btn-red {";
	st += "        background-color: #f44336;";
	st += "        box-shadow: 0 5px #a33;";
	st += "      }";
	st += "      .btn-red:hover {background-color: #d32f2f}";
	st += "      .btn-red:active {";
	st += "        background-color: #d32f2f;";
	st += "        box-shadow: 0 2px #911;";
	st += "      }";
	st += "    </style>";
	st += "  </head>";
	return st;
}



