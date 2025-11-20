
String GetPageInfo() {
    String HTML = HtmlGetHead("Module Info");
    
    HTML += "<body>\r\n";
    HTML += "<p style='text-align: center;'><b>Module Information</b></p>\r\n";
    HTML += "<hr />\r\n";
    
    // Temperature sensor
    float temp_celsius = 0;
    temp_sensor_read_celsius(&temp_celsius);
    
    HTML += "<p><b>Temperature:</b> ";
    HTML += String(temp_celsius, 1);
    HTML += " &deg;C</p>\r\n";
    
    // Current monitoring
    HTML += "<p><b>Current Monitor 1 (Pin 6):</b> ";
    HTML += String(getCurrentInAmps(Current1Pin), 2);
    HTML += " A</p>\r\n";
    
    HTML += "<p><b>Current Monitor 2 (Pin 14):</b> ";
    HTML += String(getCurrentInAmps(Current2Pin), 2);
    HTML += " A</p>\r\n";
    
    HTML += "<hr />\r\n";
    
    // Sensor information
    for (int i = 0; i < MDL.SensorCount; i++) {
        HTML += "<p><b>Sensor ";
        HTML += String(i);
        HTML += ":</b></p>\r\n";
        HTML += "<ul>\r\n";
        HTML += "<li>Flow Pin: ";
        HTML += String(Sensor[i].FlowPin);
        HTML += "</li>\r\n";
        HTML += "<li>IN1: ";
        HTML += String(Sensor[i].IN1);
        HTML += "</li>\r\n";
        HTML += "<li>IN2: ";
        HTML += String(Sensor[i].IN2);
        HTML += "</li>\r\n";
        HTML += "<li>UPM: ";
        HTML += String(Sensor[i].UPM, 2);
        HTML += "</li>\r\n";
        HTML += "<li>PWM: ";
        HTML += String(Sensor[i].PWM, 1);
        HTML += "</li>\r\n";
        HTML += "<li>Target UPM: ";
        HTML += String(Sensor[i].TargetUPM, 2);
        HTML += "</li>\r\n";
        HTML += "<li>Flow Enabled: ";
        HTML += Sensor[i].FlowEnabled ? "Yes" : "No";
        HTML += "</li>\r\n";
        HTML += "</ul>\r\n";
    }
    
    HTML += "<hr />\r\n";
    HTML += "<p><a href='/'>Return to Main Page</a></p>\r\n";
    HTML += "</body>\r\n";
    HTML += "</html>\r\n";
    
    return HTML;
}
