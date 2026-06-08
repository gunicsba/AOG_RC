String GetPage2()
{
    String st = HtmlGetHead("Network");
    // page-specific styles
    st += "    <style>";
    st += "      h1 { text-decoration: underline; }";
    st += "      h1.subhead { margin:20px auto 12px; text-decoration:none; }";
    st += "      table.center { border-collapse:collapse; table-layout:fixed; }";
    st += "      td.label-col { width:200px; text-align:left; padding:8px 12px; vertical-align:middle; }";
    st += "      td.input-col { width:320px; padding:8px 12px; vertical-align:middle; }";
    st += "      .control-width { width:320px; max-width:90%; margin:0 auto; box-sizing:border-box; }";
    st += "      .InputCell { display:block; width:100%; height:36px; box-sizing:border-box; text-align:center; font-size:18px; font-weight:700; padding:4px 6px; }";
    st += "      #submitBtn { margin-top: 36px; }";
    st += "      .checkbox-row { display:flex; align-items:center; height:44px; box-sizing:border-box; }";
    st += "      .checkbox-left { display:flex; align-items:center; justify-content:flex-start; }";
    st += "      input[type=checkbox].styled {";
    st += "        -webkit-appearance: none; appearance: none;";
    st += "        width:44px; height:44px; display:inline-block; position:relative; margin:0; padding:0; box-sizing:border-box;";
    st += "        border-radius:10px;";
    st += "        background-image: linear-gradient(rgba(179,132,201,.84), rgba(57,31,91,.84) 50%);";
    st += "        box-shadow: rgba(57,31,91,0.24) 0 2px 2px, rgba(179,132,201,0.4) 0 8px 12px;";
    st += "        cursor:pointer; vertical-align:middle; outline: none;";
    st += "        border: 1px solid rgba(57,31,91,0.25);";
    st += "      }";
    st += "      input[type=checkbox].styled::after {";
    st += "        content: ''; position: absolute; left: 50%; top: 50%;";
    st += "        width: 12px; height: 22px;";
    st += "        border-right: 4px solid white; border-bottom: 4px solid white;";
    st += "        transform: translate(-50%,-60%) rotate(45deg) scale(0);";
    st += "        transform-origin: center; transition: transform 0.12s ease-in-out;";
    st += "        border-radius:2px;";
    st += "      }";
    st += "      input[type=checkbox].styled:checked::after { transform: translate(-50%,-60%) rotate(45deg) scale(1); }";
    st += "      input[type=checkbox].styled:focus { box-shadow: 0 0 0 3px rgba(179,132,201,0.22); }";
    st += "      .label-normal { font-weight:normal; }";
    st += "      .hint { font-size: 12px; color: #333; margin-top: 4px; }";
    st += "      .status { margin: 2px auto 16px; font-size: 16px; }";
    st += "    </style>";

    st += "    <h1>Wifi Network</h1>";
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
    st += "      <p><div class='control-width'><input class='button-72' id='submitBtn' type='submit' value='Save/Restart'></div></p>";
    st += "      <p> <a href='/'>Back</a> </p>";
    st += "    </form>";
    st += "</body></html>";

    return st;
}
