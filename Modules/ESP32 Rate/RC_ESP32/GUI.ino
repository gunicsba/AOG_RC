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
  newState = server.arg("b9threlay") == "on";
  if(b9threlay != newState) {
    b9threlay = newState;
    SaveData();
  }
  
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

String GetCssStyle()
{
    String st;

    st += "    <style>";

    // ---- GLOBAL ---------------------------------------------------------
    st += "      html { font-family: Helvetica, Arial, sans-serif; display:inline-block; margin:0 auto; text-align:center; }";
    st += "      body { margin-top:50px; background-color:wheat; }";
    st += "      h1 { color:#444444; margin:50px auto 30px; }";
    st += "      a:link { font-size:150%; }";
    st += "      table.center { margin-left:auto; margin-right:auto; border-collapse:collapse; }";

    // ---- INPUT BASE -----------------------------------------------------
    st += "      .control { width:320px; max-width:90%; margin:0 auto; box-sizing:border-box; }";
    st += "      .InputCell { display:block; width:100%; height:36px; box-sizing:border-box; text-align:center; font-size:18px; font-weight:700; padding:4px 6px; }";

    // ---- BUTTON BASE ----------------------------------------------------
    st += "      .btn-base {";
    st += "        align-items:center;";
    st += "        background-color:initial;";
    st += "        border-radius:42px;";
    st += "        border-width:0;";
    st += "        box-shadow:rgba(57,31,91,0.24) 0 2px 2px, rgba(179,132,201,0.4) 0 8px 12px;";
    st += "        color:#FFFFFF;";
    st += "        cursor:pointer;";
    st += "        display:inline-flex;";
    st += "        justify-content:center;";
    st += "        font-family:Quicksand, sans-serif;";
    st += "        font-size:18px;";
    st += "        font-weight:700;";
    st += "        letter-spacing:.04em;";
    st += "        line-height:16px;";
    st += "        margin:12px auto;";
    st += "        padding:12px 18px;";
    st += "        text-align:center;";
    st += "        text-decoration:none;";
    st += "        text-shadow:rgba(255,255,255,0.4) 0 0 4px, rgba(255,255,255,0.2) 0 0 12px, rgba(57,31,91,0.6) 1px 1px 4px, rgba(57,31,91,0.32) 4px 4px 16px;";
    st += "        user-select:none;";
    st += "        width:320px;";
    st += "        max-width:90%;";
    st += "      }";

    // ---- BUTTON COLOR VARIANTS -----------------------------------------
    st += "      .btn-purple { background-image:linear-gradient(rgba(179,132,201,.84), rgba(57,31,91,.84) 50%); }";
    st += "      .btn-green  { background-image:linear-gradient(rgba(50,255,50,.84), rgba(30,150,30,.84) 50%); }";
    st += "      .btn-red    { background-image:linear-gradient(rgba(255,50,50,.84), rgba(150,30,30,.84) 50%); }";

    // ---- CHECKBOX STYLED -----------------------------------------------
    st += "      .checkbox-row { display:flex; align-items:center; height:44px; }";
    st += "      input[type=checkbox].check-styled {";
    st += "        -webkit-appearance:none; appearance:none;";
    st += "        width:44px; height:44px; display:inline-block; position:relative;";
    st += "        border-radius:10px;";
    st += "        background-image:linear-gradient(rgba(179,132,201,.84), rgba(57,31,91,.84) 50%);";
    st += "        box-shadow:rgba(57,31,91,0.24) 0 2px 2px, rgba(179,132,201,0.4) 0 8px 12px;";
    st += "        cursor:pointer; border:1px solid rgba(57,31,91,0.25);";
    st += "      }";

    st += "      input[type=checkbox].check-styled::after {";
    st += "        content:''; position:absolute;";
    st += "        left:50%; top:50%;";
    st += "        width:12px; height:22px;";
    st += "        border-right:4px solid white; border-bottom:4px solid white;";
    st += "        transform:translate(-50%,-60%) rotate(45deg) scale(0);";
    st += "        transform-origin:center;";
    st += "        transition:transform 0.12s ease-in-out;";
    st += "      }";

    st += "      input[type=checkbox].check-styled:checked::after {";
    st += "        transform:translate(-50%,-60%) rotate(45deg) scale(1);";
    st += "      }";

    st += "      input[type=checkbox].check-styled:focus { box-shadow:0 0 0 3px rgba(179,132,201,0.22); }";

    st += "    </style>";

    return st;
}




