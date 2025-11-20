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

void HandleInfo()
{
	server.send(200, "text/html", GetPageInfo());
}

void Cytron()
{
	if (server.hasArg("disableMotor"))
	{
		disableMotor = (server.arg("disableMotor") == "1");
	}
	if (server.hasArg("disableFlow"))
	{
		disableFlow = (server.arg("disableFlow") == "1");
	}
	if (server.hasArg("b9threlay"))
	{
		b9threlay = (server.arg("b9threlay") == "1");
	}
	SaveData();
	server.send(200, "text/html", GetPage0());
}

String HtmlGetHead(String title)
{
	String HTML = "<!DOCTYPE html>\r\n";
	HTML += "<html>\r\n";
	HTML += "<head>\r\n";
	HTML += "<meta name='viewport' content='width=device-width, initial-scale=1'>\r\n";
	HTML += "<title>";
	HTML += title;
	HTML += "</title>\r\n";
	HTML += "<style>\r\n";
	HTML += "body { font-family: Arial, sans-serif; margin: 20px; }\r\n";
	HTML += "hr { border: 1px solid #ccc; }\r\n";
	HTML += "</style>\r\n";
	HTML += "</head>\r\n";
	return HTML;
}

