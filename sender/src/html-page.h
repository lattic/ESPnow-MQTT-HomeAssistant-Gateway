#ifndef CHARGING_GPIO
    u_int8_t charg_gpio = -1;
#else
    u_int8_t charg_gpio = CHARGING_GPIO;
#endif
#ifndef POWER_GPIO
    u_int8_t power_gpio = -1;
#else   
    u_int8_t power_gpio = POWER_GPIO;
#endif
#ifndef ERROR_RED_LED_GPIO
    u_int8_t error_led_gpio = -1;
#else   
    u_int8_t error_led_gpio = ERROR_RED_LED_GPIO;
#endif
#ifndef ACT_BLUE_LED_GPIO
    u_int8_t activity_led_gpio = -1;
#else   
    u_int8_t activity_led_gpio = ACT_BLUE_LED_GPIO;
#endif
#ifndef ENABLE_3V_GPIO
    u_int8_t enable_3v_gpio = -1;
#else   
    u_int8_t enable_3v_gpio = ENABLE_3V_GPIO;
#endif
#ifndef FW_UPGRADE_GPIO
    u_int8_t fw_upgrade_gpio = -1;
#else   
    u_int8_t fw_upgrade_gpio = FW_UPGRADE_GPIO;
#endif

cp_html_page  =
"<!DOCTYPE HTML><html><head>\
<title>DEVICE PROVISIONING PORTAL</title>\
<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
</head>\
<style>\
#timeout {\
background-color: red;\
color: white;\
padding: 4px;\
text-align: center;\
font-size:200%;\
white-space: nowrap;\
}\
</style>\
<body bgcolor=\"#212223\">\
<h3 style=\"color: #FFFFFF\">DEVICE PROVISIONING PORTAL</p></h3><h4><p id=\"timeout\"></h4>\
<h4 style=\"color: #FFFFFF\">Device data:</h4>\
<p style=\"color: #B3B3B3\">Name:&nbsp<a style=color:yellow>";
cp_html_page += DEVICE_NAME;
cp_html_page += "</a>, Hostname:&nbsp<a style=color:yellow>";
cp_html_page += HOSTNAME;
cp_html_page += "</a>, MCU type:&nbsp<a style=color:yellow>";
cp_html_page += MODEL;
cp_html_page += "</a>, org MAC:&nbsp<a style=color:yellow>";
cp_html_page += mac_org_char;
cp_html_page += "</a>, new MAC:&nbsp<a style=color:yellow>";
cp_html_page += mac_new_char;
cp_html_page += "</a><br><a style=color:red>RED</a> LED GPIO:&nbsp<a style=color:yellow>";
cp_html_page += error_led_gpio;
cp_html_page += "</a>, <a style=color:limegreen>GREEN</a> LED GPIO:&nbsp<a style=color:yellow>";
cp_html_page += activity_led_gpio;
cp_html_page += "</a>, FW upgrade GPIO:&nbsp<a style=color:yellow>";
cp_html_page += fw_upgrade_gpio;
cp_html_page += "</a>, 3V enable GPIO:&nbsp<a style=color:yellow>";
cp_html_page += enable_3v_gpio;
cp_html_page += "</a>, Power GPIO:&nbsp<a style=color:yellow>";
cp_html_page += power_gpio;
cp_html_page += "</a>, Charging GPIO:&nbsp<a style=color:yellow>";
cp_html_page += charg_gpio;
cp_html_page += "</a><br>Firmware version:&nbsp<a style=color:yellow>";
cp_html_page += ZH_PROG_VERSION;
cp_html_page += "</a>, Compiled on:&nbsp<a style=color:yellow>";
cp_html_page += __DATE__;
cp_html_page += ", ";
cp_html_page += __TIME__;
cp_html_page += "</a><br>Total on-time:&nbsp<a style=color:yellow>";
cp_html_page += g_saved_ontime_ms/1000;
cp_html_page += "s</a>, Boot count:&nbsp<a style=color:yellow>";
cp_html_page += g_bootCount;
cp_html_page += "</a>\
<br><h4 style=\"color: #FFFFFF\">Configuration data:</h4>\
<form action=\"/get\">\
<p style=\"color: #B3B3B3\">\
SSID (<=32 characters): <input type=\"text\" name=\"ssid\" maxlength=\"32\" size=\"32\" value=\"";
cp_html_page += old_ssid;
cp_html_page += "\"\"><br>\
Password (8-63 characters): <input type=\"password\" name=\"password\" id=\"password\" maxlength=\"63\" size=\"63\" value=\"";
cp_html_page += old_password;
cp_html_page += "\"\">\
<input type=\"checkbox\" onclick=\"myFunction()\">Show Password\
<script>\
function myFunction() {\
var x = document.getElementById(\"password\");\
if (x.type === \"password\") {\
x.type = \"text\";\
} else {\
x.type = \"password\";\
}\
}\
</script>\
<br>\
Sleep time in seconds [1-86400 (24h)] (if wrong value entered, default is ";
cp_html_page += SLEEP_TIME_S;
cp_html_page += "s) <input type=\"text\" name=\"sleeptime_s_str\" maxlength=\"5\" size=\"5\" value=\"";
cp_html_page += old_sleeptime_s_str;
cp_html_page += "\"\"><br>\
<input type=\"submit\" value=\"Submit\" style=\"border: none; height:30px; width:100px font-size:50px\">\
</form>\
</br>\
<script>\
var countDownDate = new Date().getTime() + (";
cp_html_page += CP_TIMEOUT_S;
cp_html_page += "*1000);\
var x = setInterval(function() {\
var now = new Date().getTime();\
var distance = countDownDate - now;\
var minutes = Math.floor((distance % (1000 * 60 * 60)) / (1000 * 60));\
var seconds = Math.floor((distance % (1000 * 60)) / 1000);\
document.getElementById(\"timeout\").innerHTML = \"Timeout: \" + minutes + \"m \" + seconds + \"s \";\
if (distance < 0) {\
clearInterval(x);\
document.getElementById(\"demo\").innerHTML = \"EXPIRED\";\
}\
}, 500);\
</script>\
</body></html>";     
