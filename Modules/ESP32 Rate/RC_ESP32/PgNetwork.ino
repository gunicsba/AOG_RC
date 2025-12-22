String GetPage2()
{
    String st = "<HTML>";
    st += "  <head>";
    st += "    <META content='text/html; charset=utf-8' http-equiv=Content-Type>";
    st += "    <meta name=vs_targetSchema content='HTML 4.0'>";
    st += "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    st += "    <title>Rate Control</title>";
    st += GetCssStyle();
    st += "  </head>";
    st += "";
    st += "  <BODY>";
    st += "    <h1 align=center>Wifi Network </h1>";
    st += "    <form id=FORM1 method=post action='/'>&nbsp;";
    st += "      <table class='center'>";
    st += "        <tr>";
    st += "          <td class='label-col'><span class='label-normal'>Network</span></td>";
    st += "          <td class='input-col'><div class='control-width'><input class='InputCell' id='ssid' size='20' name='prop1' value='" + String(MDLnetwork.SSID) + "'></div></td>";
    st += "        </tr>";
    st += "        <tr>";
    st += "          <td class='label-col'><span class='label-normal'>Password</span></td>";
    st += "          <td class='input-col'><div class='control-width'><input class='InputCell' id='pass' size='20' name='prop2' value='" + String(MDLnetwork.Password) + "'></div></td>";
    st += "        </tr>";
    st += "        <tr>";
    st += "          <td class='label-col'><span class='label-normal'>Use this Network</span></td>";
    st += "          <td class='input-col'>";
    st += "            <div class='control-width'>";
    st += "              <div class='checkbox-row'>";
    st += "                <div class='checkbox-left'>";
    st += "                  <label style='display:inline-flex; align-items:center; gap:8px; cursor:pointer;'>";
    st += "                    <input class='styled' type='checkbox' name='connect' value='1' " + String(MDLnetwork.WifiModeUseStation ? "checked" : "") + ">";
    st += "                  </label>";
    st += "                </div>";
    st += "              </div>";
    st += "            </div>";
    st += "          </td>";
    st += "        </tr>";
    // WiFi status row centered across page
    st += "        <tr><td colspan='2' style='text-align:center;'>";
    if (WiFi.isConnected())
    {
        st += "<div class='status'>Wifi Connected to " + String(MDLnetwork.SSID) + "</div>";
    }
    else
    {
        st += "<div class='status'>Wifi Not Connected</div>";
    }
    st += "        </td></tr>";
    st += "        <tr><td colspan='2'><hr></td></tr>";
    // New Hotspot heading row (renamed and underlined via h1 style) with zero td padding to match spacing
    st += "        <tr><td colspan='2' style='text-align:center; padding:0;'><h1 class='subhead'>Hotspot</h1></td></tr>";
    st += "        <tr>";
    st += "          <td class='label-col'><span class='label-normal'>Password</span></td>";
    st += "          <td class='input-col'><div class='control-width'><input class='InputCell' id='ap_pass' size='20' name='prop3' value='" + String(MDL.APpassword) + "'></div></td>";
    st += "        </tr>";
    st += "        <tr>";
    st += "          <td colspan='2'><div class='control-width'><div class='hint'>Module Access Point. Use 8 to 10 characters. Leave empty for an open hotspot.</div></div></td>";
    st += "        </tr>";
    st += "      </table>";
    st += "";
    st += "      <p><div class='control-width'><input class='btn-base btn-purple' id='submitBtn' type='submit' value='Save/Restart'></div></p>";
    st += "      <p> <a href='/page0'>Back</a> </p>";
    st += "    </form>";
    st += "  </BODY>";
    st += "</HTML>";

    return st;
}
