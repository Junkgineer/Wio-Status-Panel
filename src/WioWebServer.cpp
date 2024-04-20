#include "WioWebServer.h"
// #include <stdint.h>
// #include "String.h"
// #include <WebServer.h>
// #include <HTTPClient.h>

WebServer webserver(80);
Temperature serv_temperature;
AirQuality serv_airquality;
IPAddress server_ip;
bool available;
bool use_custom;
String *c_status;
uint8_t *b_minutes;
double temp;
double humidity;
uint16_t tvoc;
uint16_t eco2;
bool *serv_is_busy;
double test_Temp = 99.9;
double test_Humidity = 99.9;
int test_TVOC = 9999;
int test_eCO2 = 999;
String index_page = "<!DOCTYPE HTML><html><head><style>body {background-color: #161719;font-family: Arial, Helvetica, sans-serif;color: #DEDEDE;}.card {background-color: #282830;min-width: 200px;min-height: 200px;margin: 10px;border: 2px solid #171724;border-radius: 10px;flex: 1;}.card:hover {background-color: #3F3F4C;min-width: 200px;min-height: 200px;margin: 10px;border: 2px solid #171724;border-radius: 5px;flex: 1;}.card-header {background-color: #696979;width: 100;height: 30px;box-shadow: 2px 2px 5px #05050575;border-radius: 5px;}.card-header-2 {margin-left: 8px;margin-right: 16px;padding-bottom: 4px;border-bottom: 1px solid #5A5DFF88;}.header-text-2 {color: #DEDEDE;margin: auto;padding-top: 8px;padding-left: 8px;text-align: left;vertical-align: middle;font-size: 12px;}.header-text {color: #161719;margin: auto;padding-top: 8px;padding-left: 8px;text-align: left;vertical-align: middle;font-size: 12px;}.large-text {font-size: 72px;}.item-text {text-align: center;vertical-align: middle;color: #DEDEDE;}.sub-text {position: relative;text-align: center;color: #DEDEDE;font-size: 16px;}.warn {color: #FFE758;}.alarm {color: #C85AF3;}.btn-holder {margin: 8px;padding-top: 16px;padding-bottom: 16px;height: 30px;justify-content: center;vertical-align: text-bottom;display: flex;}.switch {position: relative;display: inline-block;width: 60px;height: 34px;}.switch input {opacity: 0;width: 0;height: 0;}.slider {position: absolute;cursor: pointer;top: 0;left: 0;right: 0;bottom: 0;background-color: #ccc;-webkit-transition: .4s;transition: .4s;}.slider:before {position: absolute;content: \"\";height: 26px;width: 26px;left: 4px;bottom: 4px;background-color: white;-webkit-transition: .4s;transition: .4s;}input:checked+.slider {background-color: #C85AF3;}input:focus+.slider {box-shadow: 0 0 1px #C85AF3;}input:checked+.slider:before {-webkit-transform: translateX(26px);-ms-transform: translateX(26px);transform: translateX(26px);}/* Rounded sliders */.slider.round {border-radius: 34px;}.slider.round:before {border-radius: 50%%;}</style></head><body><div style=\"display: flex; width: 75; margin: auto;\"><div class=\"card\"><div class=\"card-header\"><h3 class=\"header-text\">Status</h3></div><h1 id=\"status-label\" class=\"item-text\">%s</h1><div class=\"btn-holder\"> <div class=\"btn-holder\"> <form id=\"statusForm\" action=\"/update\"><label class=\"switch\"> <input id=\"status\" name=\"status\" type=\"checkbox\" value=\"%s\" ><span class=\"slider round\"></span></label></form></div></div></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">Temperature</h3></div><h1 class=\"item-text large-text\">%1.1f<div class=\"sub-text\">Fahrenheit</div></h1></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">Humidity</h3></div><h1 class=\"item-text warn large-text\">%1.1f</h1></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">TVOC</h3></div><h1 class=\"item-text alarm large-text\">%d<div class=\"sub-text\">PPB</div></h1></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">eCo2</h3></div><h1 class=\"item-text large-text\">%d<div class=\"sub-text\">PPM</div></h1></div></div><script>const statusLabel = document.getElementById('status-label');const statusToggle = document.getElementById('status');statusToggle.value = statusLabel.textContent;if (statusLabel.textContent == 'Busy') {statusToggle.checked = true;statusToggle.value = 'Busy';} else {statusToggle.checked = false;statusToggle.value = 'Available';}statusToggle.addEventListener('click', () => { if (statusLabel.textContent == 'Available') { statusLabel.textContent == 'Busy';statusToggle.value = 'Busy' ;statusToggle.checked = true;} else { statusLabel.textContent == 'Available';statusToggle.value = 'Available' ;statusToggle.checked = false;}document.getElementById('statusForm').submit()});</script></body></html>";

WioWebServer::WioWebServer()
{
}
// Both of these take an "editable" copy of Temperature object, assigns it directly to a
//  private pointer in the class, and then creates a local copy for non-members to use.
// TODO: Is this any better than just passing a pointer to begin with? I don't think I plan on
//  making any changes to the objects, so I doubt a copy is needed. Recheck this later.
WioWebServer::WioWebServer(Temperature &temp) : _temperature(&temp)
{
    serv_temperature = *_temperature;
    serv_airquality = AirQuality();
}
WioWebServer::WioWebServer(Temperature &temp, AirQuality &air, String &status, uint8_t &minutes, bool &busy, IPAddress &ip) : _temperature(&temp), _airquality(&air), _current_status(&status), _busy_minutes(&minutes), _is_busy(&busy), _server_IP(&ip)
{
    serv_temperature = *_temperature;
    serv_airquality = *_airquality;
    server_ip = *_server_IP;
    c_status = _current_status;
    b_minutes = _busy_minutes;
    serv_is_busy = _is_busy;
}
void WioWebServer::UseTestPage() {
    Current_Settings.index_page = "<!DOCTYPE HTML><html><head><style>body {background-color: #161719;font-family: Arial, Helvetica, sans-serif;color: #DEDEDE;}.card {background-color: #282830;min-width: 200px;min-height: 200px;margin: 10px;border: 2px solid #171724;border-radius: 10px;flex: 1;}.card:hover {background-color: #3F3F4C;min-width: 200px;min-height: 200px;margin: 10px;border: 2px solid #171724;border-radius: 5px;flex: 1;}.card-header {background-color: #696979;width: 100;height: 30px;box-shadow: 2px 2px 5px #05050575;border-radius: 5px;}.card-header-2 {margin-left: 8px;margin-right: 16px;padding-bottom: 4px;border-bottom: 1px solid #5A5DFF88;}.header-text-2 {color: #DEDEDE;margin: auto;padding-top: 8px;padding-left: 8px;text-align: left;vertical-align: middle;font-size: 12px;}.header-text {color: #161719;margin: auto;padding-top: 8px;padding-left: 8px;text-align: left;vertical-align: middle;font-size: 12px;}.large-text {font-size: 72px;}.item-text {text-align: center;vertical-align: middle;color: #DEDEDE;}.sub-text {position: relative;text-align: center;color: #DEDEDE;font-size: 16px;}.warn {color: #FFE758;}.alarm {color: #C85AF3;}.btn-holder {margin: 8px;padding-top: 16px;padding-bottom: 16px;height: 30px;justify-content: center;vertical-align: text-bottom;display: flex;}.switch {position: relative;display: inline-block;width: 60px;height: 34px;}.switch input {opacity: 0;width: 0;height: 0;}.slider {position: absolute;cursor: pointer;top: 0;left: 0;right: 0;bottom: 0;background-color: #ccc;-webkit-transition: .4s;transition: .4s;}.slider:before {position: absolute;content: \"\";height: 26px;width: 26px;left: 4px;bottom: 4px;background-color: white;-webkit-transition: .4s;transition: .4s;}input:checked+.slider {background-color: #C85AF3;}input:focus+.slider {box-shadow: 0 0 1px #C85AF3;}input:checked+.slider:before {-webkit-transform: translateX(26px);-ms-transform: translateX(26px);transform: translateX(26px);}/* Rounded sliders */.slider.round {border-radius: 34px;}.slider.round:before {border-radius: 50%%;}</style></head><body><div style=\"display: flex; width: 75; margin: auto;\"><div class=\"card\"><div class=\"card-header\"><h3 class=\"header-text\">Status</h3></div><h1 id=\"status-label\" class=\"item-text\">%s</h1><div class=\"btn-holder\"> <div class=\"btn-holder\"> <form id=\"statusForm\" action=\"/update\"><label class=\"switch\"> <input id=\"status\" name=\"status\" type=\"checkbox\" value=\"%s\" ><span class=\"slider round\"></span></label></form></div></div></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">Temperature</h3></div><h1 class=\"item-text large-text\">%1.1f<div class=\"sub-text\">Fahrenheit</div></h1></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">Humidity</h3></div><h1 class=\"item-text warn large-text\">%1.1f</h1></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">TVOC</h3></div><h1 class=\"item-text alarm large-text\">%d<div class=\"sub-text\">PPB</div></h1></div><div class=\"card\"><div class=\"card-header-2\"><h3 class=\"header-text-2\">eCo2</h3></div><h1 class=\"item-text large-text\">%d<div class=\"sub-text\">PPM</div></h1></div></div><script>const statusLabel = document.getElementById('status-label');const statusToggle = document.getElementById('status');statusToggle.value = statusLabel.textContent;if (statusLabel.textContent == 'Busy') {statusToggle.checked = true;statusToggle.value = 'Busy';} else {statusToggle.checked = false;statusToggle.value = 'Available';}statusToggle.addEventListener('click', () => { if (statusLabel.textContent == 'Available') { statusLabel.textContent == 'Busy';statusToggle.value = 'Busy' ;statusToggle.checked = true;} else { statusLabel.textContent == 'Available';statusToggle.value = 'Available' ;statusToggle.checked = false;}document.getElementById('statusForm').submit()});</script></body></html>";
}
void DumpToSerial()
{
    char outbuf[6];
    //Serial.println("Status: " + *c_status);
    snprintf(outbuf, sizeof(outbuf), "%06f", serv_temperature.current_temperature);
    //Serial.println("Temperature: " + (String)outbuf);
    snprintf(outbuf, sizeof(outbuf), "%06f", serv_temperature.humidity);
    //Serial.println("Humidity %: " + (String)outbuf);
    snprintf(outbuf, sizeof(outbuf), "%06u", serv_airquality.tvoc);
    //Serial.println("TVOC ppb: " + (String)outbuf);
    snprintf(outbuf, sizeof(outbuf), "%06u", serv_airquality.eco2);
    //Serial.println("eCO2 ppm: " + (String)outbuf);
}
// String BuildPage(String *status, double *temp, double *humidity, uint16_t *tvoc, uint16_t *eco2)
void BuildPage()
{
    if (*serv_is_busy)
    {
        *c_status = "Busy";
    }
    else
    {
        *c_status = "Available";
    }
    if (serv_temperature.is_error)
    {
        temp = 99;
        humidity = 99;
    }
    else
    {
        temp = serv_temperature.current_temperature;
        humidity = serv_temperature.humidity;
    }
    if (serv_airquality.is_error)
    {
        tvoc = 9999;
        eco2 = 999;
    }
    else
    {
        tvoc = serv_airquality.tvoc;
        eco2 = serv_airquality.eco2;
    }

    char ip_buffer[16];
    snprintf(ip_buffer, sizeof(ip_buffer), "%d.%d.%d.%d", server_ip[0], server_ip[1], server_ip[2], server_ip[3]);

    char statBuf[15];
    c_status->toCharArray(statBuf, c_status->length() + 1);
    //Serial.println("-----|Page Data|-----");
    //Serial.println("IP: " + (String)ip_buffer);
    // Serial.print("Status: ");
    //Serial.println(statBuf);
    // Serial.print("Temp: ");
    //Serial.println(temp);
    // Serial.print("Humidity: ");
    //Serial.println(humidity);
    // Serial.print("TVOC: ");
    //Serial.println(tvoc);
    // Serial.print("eCO2: ");
    //Serial.println(eco2);
    //Serial.println("---------------------");
    if (use_custom) {
        //Serial.println("Using custom index page...");
    }
    int html_len = index_page.length() + 1;
    html_len += sizeof(statBuf)*2;
    html_len += 20;
    char html[html_len];
    char page[html_len];
    index_page.toCharArray(html, html_len);
    Serial.print("html_len: "); //Serial.println(html_len);
    // Serial.print("index_page: "); //Serial.println(index_page);

    // snprintf(page, sizeof(page), html, statBuf, statBuf, temp, humidity, tvoc, eco2);

    snprintf(page, sizeof(page), html, statBuf, temp, humidity, tvoc, eco2);
    
    webserver.send(200, "text/html", page);
}
void handleRoot()
{
    //Serial.println("Handling root web request...");
    BuildPage();
}
void handleUpdate()
{
    // http://192.168.10.154/update?status=busy&minutes=32
    //Serial.println("Handling status update request...");
    char *stat[15];
    //Serial.println("Arguments passed:");
    // if (*serv_is_busy && webserver.args() == 0) {
    //     *serv_is_busy = false;
    // }
    if (webserver.args() == 0) {
        *serv_is_busy = false;
    }
    for (uint8_t i = 0; i < webserver.args(); i++)
    {
        //Serial.println(webserver.argName(i));
        if (webserver.argName(i) == "status")
        {
            //Serial.println("Arg value: " + webserver.arg(i));
            if (webserver.arg(i) == "busy" || webserver.arg(i) == "Busy")
            {
                *serv_is_busy = true;
            }
            else
            {
                *serv_is_busy = false;
            }
        }
        else if (webserver.argName(i) == "minutes")
        {
            // *b_minutes = webserver.arg(i);
            //Serial.println("Minutes rec: " + webserver.arg(i));
        }
    }
    BuildPage();
}
void handleAPI()
{
    Serial.println("Handling API...");
    // DumpToSerial();
    uint8_t temp_error;
    temp_error = 0;
    char tempbuf[3];
    char humbuf[3];
    uint8_t air_error;
    air_error = 0;
    char tvocbuf[6];
    char eco2buf[6];
    if (serv_temperature.is_error)
    {
        snprintf(tempbuf, sizeof(tempbuf), "%03f", test_Temp);
        snprintf(humbuf, sizeof(humbuf), "%03f", test_Humidity);
        temp_error = 1;
    }
    else
    {
        snprintf(tempbuf, sizeof(tempbuf), "%03f", serv_temperature.current_temperature);
        snprintf(humbuf, sizeof(humbuf), "%03f", serv_temperature.humidity);
        temp_error = 0;
    }
    if (serv_airquality.is_error)
    {
        snprintf(tvocbuf, sizeof(tvocbuf), "%03d", test_TVOC);
        snprintf(eco2buf, sizeof(eco2buf), "%03d", test_eCO2);
        air_error = 1;
    }
    else
    {
        snprintf(tvocbuf, sizeof(tvocbuf), "%04u", serv_airquality.tvoc);
        snprintf(eco2buf, sizeof(eco2buf), "%04u", serv_airquality.eco2);
        air_error = 0;
    }
    // char msg[200] = "Not implemented";
    char statBuf[15];
    c_status->toCharArray(statBuf, c_status->length() + 1);

    String json_msg = "{\"status\": \"%s\", \"temperature\": %d, \"humidity\": %d, \"tvoc\": %d, \"eco2\": %d}";

    int json_len = json_msg.length() + 1;
    json_len += sizeof(statBuf)*2;
    json_len += 20;
    char msg[json_len];
    char rmsg[json_len];
    json_msg.toCharArray(msg, json_len);
    snprintf(rmsg, sizeof(rmsg), msg, statBuf, temp, humidity, tvoc, eco2);
    // snprintf(tempbuf, sizeof(tempbuf), "%03f", serv_temperature.current_temperature);
    webserver.send(200, "application/json", rmsg);
}
void handleNotFound()
{
    String message = "URL Not Found\n\n";
    message += "URI: ";
    message += webserver.uri();
    message += "\nMethod: ";
    message += (webserver.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += webserver.args();
    message += "\n";
    for (uint8_t i = 0; i < webserver.args(); i++)
    {
        message += " " + webserver.argName(i) + ": " + webserver.arg(i) + "\n";
    }
    webserver.send(404, "text/plain", message);
    //Serial.println("Webserver error:");
    //Serial.println(message);
}

void WioWebServer::UpdateStatus(String *status, uint8_t *busy_mins)
{
    // WioWebServer::_current_status = &status;
    // b_minutes = busy_mins;
}
void WioWebServer::SetMinutes(uint8_t *busy_mins)
{
    b_minutes = busy_mins;
}
uint8_t WioWebServer::begin()
{
    if (Current_Settings.webserv_use_custom) {
        use_custom = Current_Settings.webserv_use_custom;
        index_page = Current_Settings.index_page;
    } else {
        use_custom = false;
    }
    webserver.on("/", handleRoot);
    // webserver.on("/", handleAPI);
    webserver.on("/update", handleUpdate);
    webserver.on("/api", handleAPI);
    webserver.onNotFound(handleNotFound);

    // webserver->on("/api", WioWebServer::API);
    //   webserver.onNotFound(handleNotFound);
    // const char* www_username = "admin";
    // const char* www_password = "password";
    // webserver->on("/admin", []() {
    //   if (!webserver->authenticate("admin", "password")) {
    //     return webserver->requestAuthentication();
    //   }
    //   webserver->send(200, "text/plain", "Login OK");
    // });
    webserver.begin();
    WioWebServer::is_ready = true;
    return 1;
}
void WioWebServer::Serve()
{
    if (WioWebServer::is_ready)
    {
        webserver.handleClient();
    }
}