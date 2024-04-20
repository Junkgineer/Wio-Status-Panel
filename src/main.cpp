#include <Arduino.h>
#include <TFT_eSPI.h> // Hardware-specific library
#include <SPI.h>
#include <Seeed_FS.h>
#include "SD/Seeed_SD.h"
#include <SDConfig.h>
#include "Free_Fonts.h"
#include <rpcWiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include "Network.h"
#include "NTP.h"
#include "WioWebServer.h"
#include "AirQuality.h"
// #include "WioMQTT.h"

Temperature temperature = Temperature();
AirQuality airquality = AirQuality();

Network network = Network();
NTP ntp = NTP();
WioWebServer wiowebserver = WioWebServer();
WioWebServer::Settings webserver_settings;
// WioMQTT wiomqtt;

// TFT Inits
TFT_eSPI tft = TFT_eSPI();

#define LCD_BACKLIGHT (72Ul)
// NOTE: NVIC_SystemReset();      // Resets Wio Terminal

File secrets;
const char *secretsFileName = "config.cfg";
// char* wifi_ssid;
// char* wifi_pass;

int status = WL_IDLE_STATUS; // the Wifi radio's status
int reqCount = 0;            // number of requests received

// Custom Colors
// #define BG_COLOR 0x7710        // 1e1e1eff
// #define BG_COLOR TFT_DARKGREY
#define BG_COLOR TFT_BLACK

#define TEXT_HEIGHT 16 // Height of text to be printed and scrolled
#define TEXT_WIDTH 7
#define LARGE_TEXT_HEIGHT 18
#define LARGE_TEXT_WIDTH 12

bool A_pressed = false;
bool B_pressed = false;
bool C_pressed = false;
bool UP_pressed = false;
bool DOWN_pressed = false;
bool LEFT_pressed = false;
bool RIGHT_pressed = false;
bool CENTER_pressed = false;
bool BD2_pressed = false;
bool BD3_pressed = false;
bool BD4_pressed = false;
bool BD5_pressed = false;

bool enable_serial_logging = true;
bool nightMode = false;
bool sensorsActive = true;
bool muteWarning = false;
bool muteAlarm = false;
bool ntpErrorCleared = false;

// Config default settings
char *bootup_title_text = "| J U N K G I N E E R I N G |";

uint32_t timer = millis();
uint32_t refreshTimer = millis();
uint32_t initTimer = millis();
uint32_t tempTimer = millis();
uint32_t clockTimer = millis();
uint32_t wifiTimer = millis();
uint32_t ntpTimer = millis();
uint32_t alertTimer = millis(); // The interval between each repeated sounding of the alert.
uint32_t muteAlertTimer = millis(); // If interval to keep the alert muted after it's been muted.

int yDraw = 2;
int xPos = 3;

IPAddress ip;
bool pulse = false;
bool initializing = true;
bool redraw_wifi = false;
bool redraw_ntp = false;

String current_status = "Available";
String last_status = "None";
uint8_t busy_minutes = 60;
bool is_busy = false;

void playTone(int tone, int duration)
{
    for (long i = 0; i < duration * 1000L; i += tone * 2)
    {
        digitalWrite(WIO_BUZZER, HIGH);
        delayMicroseconds(tone);
        digitalWrite(WIO_BUZZER, LOW);
        delayMicroseconds(tone);
    }
}
void confirmUpTone() {
    playTone(1519, 100);
    playTone(1014, 100);
}
void confirmDownTone() {
    playTone(1014, 100);
    playTone(1519, 100);
}
void upTone()
{
    playTone(1519, 50);
    playTone(1432, 50);
    playTone(1275, 50);
    playTone(1136, 50);
}
void downTone()
{
    playTone(1136, 50);
    playTone(1275, 50);
    playTone(1432, 50);
    playTone(1519, 50);
}
void alarmTone()
{
    for (int i=0; i<3; i++){
        playTone(1519, 500);
        playTone(1136, 500);
    }
}
void warnTone()
{
    for (int x=0;x<3;x++){
        playTone(1014, 500);
        playTone(1519, 200);
    }
}
void waveTone()
{
    int freq = 1519;
    for (int x=0;x<3;x++){
        for (int i=0; i<5; i++){ 
            playTone(freq, 100);
            freq += 100;
        }
        for (int i=0; i<5; i++){
            playTone(freq, 100);
            freq -= 100;
        }
    }
}

boolean readConfiguration()
{
    /*
     * Length of the longest line expected in the config file.
     * The larger this number, the more memory is used
     * to read the file.
     * You probably won't need to change this number.
     */
    int maxLineLength = 127;
    SDConfig cfg;
    // Open the configuration file.
    if (!cfg.begin(secretsFileName, maxLineLength))
    {
        Serial.print("Failed to open configuration file: ");
        Serial.println(secretsFileName);
        return false;
    }
    // Read each setting from the file.
    while (cfg.readNextSetting())
    {
        if (cfg.nameIs("enable_serial_logging"))
        {
            network.Current_Settings.wifi_ssid = cfg.copyValue();
        }
        if (cfg.nameIs("wifi_enable"))
        {
            network.Current_Settings.wifi_enable = cfg.copyValue();
        }
        else if (cfg.nameIs("wifi_ssid"))
        {
            network.Current_Settings.wifi_ssid = cfg.copyValue();
        }
        else if (cfg.nameIs("wifi_pass"))
        {
            // network.Current_Settings.wifi_pass = "password";
            network.Current_Settings.wifi_pass = cfg.copyValue();
        }
        else if (cfg.nameIs("wifi_use_advanced"))
        {
            network.Current_Settings.use_advanced = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("wifi_static_ip"))
        {
            network.Current_Settings.static_ip.fromString(cfg.copyValue());
        }
        else if (cfg.nameIs("wifi_gateway_ip"))
        {
            network.Current_Settings.gateway_ip.fromString(cfg.copyValue());
        }
        else if (cfg.nameIs("wifi_subnet_ip"))
        {
            network.Current_Settings.subnet_ip.fromString(cfg.copyValue());
        }        
        else if (cfg.nameIs("wifi_primary_dns_ip"))
        {
            network.Current_Settings.primary_dns_ip.fromString(cfg.copyValue());
        }
        else if (cfg.nameIs("wifi_secondary_dns_ip"))
        {
            network.Current_Settings.secondary_dns_ip.fromString(cfg.copyValue());
        }
        else if (cfg.nameIs("wifi_retry"))
        {
            // network.Current_Settings.wifi_retry = true;
            network.Current_Settings.wifi_retry = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("wifi_retry_interval_s"))
        {
            // network.Current_Settings.wifi_retry_interval_s = 120;
            network.Current_Settings.wifi_retry_interval_s = cfg.getIntValue();
        }
        else if (cfg.nameIs("webserv_enable"))
        {
            webserver_settings.webserv_enable = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("webserv_use_custom"))
        {
            webserver_settings.webserv_use_custom = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("ntp_server1"))
        {
            ntp.Current_Settings.ntp_server1 = cfg.copyValue();
        }
        else if (cfg.nameIs("ntp_server2"))
        {
            ntp.Current_Settings.ntp_server2 = cfg.copyValue();
        }
        else if (cfg.nameIs("ntp_server3"))
        {
            ntp.Current_Settings.ntp_server3 = cfg.copyValue();
        }
        else if (cfg.nameIs("ntp_enable"))
        {
            ntp.Current_Settings.ntp_enable = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("ntp_retry"))
        {
            ntp.Current_Settings.ntp_retry = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("ntp_retry_interval_s"))
        {
            ntp.Current_Settings.ntp_retry_interval_s = cfg.getIntValue();
        }
        else if (cfg.nameIs("timezone_offset_s"))
        {
            ntp.Current_Settings.timezone_offset_s = cfg.getIntValue();
        }
        else if (cfg.nameIs("enable_temperature"))
        {
            temperature.Current_Settings.enable_temperature = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("is_fahrenheit"))
        {
            temperature.Current_Settings.is_fahrenheit = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("enable_temp_alarms"))
        {
            temperature.Current_Settings.enable_temp_alarms = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("high_temp_alarm"))
        {
            temperature.Current_Settings.high_temp_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("read_interval_s"))
        {
            temperature.Current_Settings.read_interval_s = cfg.getIntValue();
        }
        else if (cfg.nameIs("low_temp_alarm"))
        {
            temperature.Current_Settings.low_temp_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("heat_index_alarm"))
        {
            temperature.Current_Settings.heat_index_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("enable_humidity_alarms"))
        {
            temperature.Current_Settings.enable_humidity_alarms = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("high_humidity_alarm"))
        {
            temperature.Current_Settings.high_humidity_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("warn_humidity_alarm"))
        {
            temperature.Current_Settings.warn_humidity_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("low_humidity_alarm"))
        {
            temperature.Current_Settings.low_humidity_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("enable_air_quality"))
        {
            airquality.Current_Settings.enable_air_quality = cfg.getBooleanValue();
        }
        else if (cfg.nameIs("air_quality_interval_s"))
        {
            airquality.Current_Settings.air_quality_interval_s = cfg.getIntValue();
        }
        else if (cfg.nameIs("high_tvoc_alarm"))
        {
            airquality.Current_Settings.high_tvoc_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("warning_tvoc_alarm"))
        {
            airquality.Current_Settings.warning_tvoc_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("low_tvoc_alarm"))
        {
            airquality.Current_Settings.low_tvoc_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("high_eco2_alarm"))
        {
            airquality.Current_Settings.high_eco2_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("warning_eco2_alarm"))
        {
            airquality.Current_Settings.warning_eco2_alarm = cfg.getIntValue();
        }
        else if (cfg.nameIs("low_eco2_alarm"))
        {
            airquality.Current_Settings.low_eco2_alarm = cfg.getIntValue();
        }
    }
    cfg.end();
    return true;
}
void NetworkStats()
{
    // Test if were booting up or not
    if (initializing)
    {
        if (network.ConnectWifi(network.Current_Settings.wifi_ssid, network.Current_Settings.wifi_pass))
        {
            tft.drawString("SUCCESS.", 196, yDraw, 2);
        }
        else
        {
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("FAIL!", 196, yDraw, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
            ntp.Current_Settings.ntp_wifi_ok = false;
        }
    } // If wifi is disabled, just draw that to the screen and continue on.
    else if (!network.Current_Settings.wifi_enable && network.update)
    {
        tft.loadFont("Orbitron-Black-12");
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString((String) "WIFI DISABLED", 3, 3, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }
    else
    {
        tft.loadFont("Orbitron-Black-12");
        tft.setTextColor(TFT_CYAN, BG_COLOR);
        // Test whether we need to draw the IP address to the screen or not.
        if (network.isConnected && ip != network.IP)
        {
            ip = network.IP;
            char ip_buffer[16];
            snprintf(ip_buffer, sizeof(ip_buffer), "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
            tft.drawString((String) "IP: " + ip_buffer, 3, 3, 2);
            // Give the IP address to the webserver
            wiowebserver.ServerIP = network.IP;
        }
        else if (!network.isConnected && network.update)
        {
            tft.loadFont("Orbitron-Black-12");
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("NO NETWORK", 3, 3, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
            network.update = false;
        }
        else if (!network.isConnected)
        {
            // Test whether we're supposed to try and reconnect on disconnect
            if (network.Current_Settings.wifi_retry)
            {
                // Check whether the retry interval has passed
                if (millis() - wifiTimer > (network.Current_Settings.wifi_retry_interval_s * 1000))
                {
                    Serial.println("Retrying Wifi...");
                    tft.fillRect(0, 0, 159, 18, BG_COLOR);
                    tft.setTextColor(TFT_YELLOW, BG_COLOR);
                    tft.drawString((String) "Retrying wifi...", 3, 3, 2);
                    tft.setTextColor(TFT_CYAN, BG_COLOR);
                    tft.fillCircle(310, 6, 5, TFT_MAGENTA);

                    if (network.Retry()) {
                        ntp.Current_Settings.ntp_wifi_ok = true;
                        wiowebserver.Current_Settings.webserv_wifi_ok = true;
                    }
                    else
                    {
                        tft.fillRect(0, 0, 159, 18, BG_COLOR);
                        tft.setTextColor(TFT_YELLOW, BG_COLOR);
                        tft.drawString((String) "Waiting to retry...", 3, 3, 2);
                        tft.setTextColor(TFT_CYAN, BG_COLOR);
                    }
                    tft.fillCircle(310, 6, 5, BG_COLOR);
                    wifiTimer = millis();
                }
            }
        }
    }
    tft.unloadFont();
}
void MQTTStats()
{
    tft.loadFont("Orbitron-Black-12");
    tft.setTextColor(TFT_CYAN, BG_COLOR);
    // if (wiomqtt.isConnected) {
    //   tft.drawString((String)"MQTT", 240, 3, 2);
    // } else {
    //   wiomqtt.Reconnect();
    //   tft.setTextColor(TFT_YELLOW, BG_COLOR);
    //   tft.drawString((String)"MQTT", 240, 3, 2);
    // }
    tft.unloadFont();
}
void ClearErrorArea() {
    tft.fillRect(160, 0, 150, 18, BG_COLOR);
}
void clearTextArea()
{
    tft.fillRect(0, 15, 320, 155, BG_COLOR);
    xPos = 3;
    yDraw = TEXT_HEIGHT + 2;
}
void ClearInfoArea()
{
    tft.fillRect(2, 23, 320, 40, BG_COLOR);
}
void clearSensorText(int boxnum = 0)
{
    switch (boxnum)
    {
        case 0:
            tft.fillRect(0, 175, 320, 240, BG_COLOR);   // Entire Row
            break;
        case 1:
            tft.fillRect(0, 177, 78, 40, BG_COLOR);     // Far left
            break;
        case 2:
            tft.fillRect(81, 177, 78, 40, BG_COLOR);    // Middle Left
            break;
        case 3:
            tft.fillRect(161, 177, 78, 40, BG_COLOR);   // Middle Right
            break;
        case 4:
            tft.fillRect(241, 177, 78, 40, BG_COLOR);   // Far Right
            break;
    }
}
uint16_t ColorToUint32(byte r, byte g, byte b)
{
    uint16_t result =
        (r << 16) + // 33
        (g << 8) +  // 33
        b;          // 33
    return result;
}
void Clock()
{
    if (!ntp.Current_Settings.ntp_enable)
    {
        Serial.println('NTP is disabled...');
        tft.loadFont("Orbitron-Black-12");
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("NTP DISABLED", 160, 3, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }
    else if (!ntp.error && ntp.Current_Settings.ntp_wifi_ok)
    {
        Serial.println("Fetching time from NTP server...");
        if (ntp.getTime()) {
            // Top error header clear
            tft.fillRect(158, 0, 85, 18, BG_COLOR);
            String c_time = ntp.Current_Time;
            if (is_busy)
            {
                tft.fillRect(96, 15, 320, 80, BG_COLOR);
                tft.loadFont("Orbitron-Black-48");
                tft.drawCentreString(c_time.substring(0, 5), 220, 40, 2);
                tft.unloadFont();

                tft.loadFont("Orbitron-Black-24");
                tft.setTextColor(TFT_YELLOW, BG_COLOR);
                tft.fillRect(0, 125, 150, 38, BG_COLOR);
                tft.drawString((String)wiowebserver.busy_minutes, 30, 140, 2);
                tft.setTextColor(TFT_CYAN, BG_COLOR);
            }
            else
            {
                tft.loadFont("Orbitron-Black-72");
                tft.fillRect(0, 15, 320, 105, BG_COLOR);
                tft.setTextColor(TFT_CYAN, BG_COLOR);
                tft.drawCentreString(c_time.substring(0, 5), 160, 60, 2);
            }
        }
    }
    else if (ntp.error)
    {
        // String c_time = ntp.Current_Time; // Should contain "UPDATE ERROR"
        ClearErrorArea();
        tft.loadFont("Orbitron-Black-12");
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("NTP ERROR", 160, 3, 2);

        clearTextArea();
        tft.setTextColor(TFT_CYAN, BG_COLOR);
        tft.loadFont("Orbitron-Black-48");
        tft.drawCentreString(current_status, 160, 80, 2);

        if (millis() - ntpTimer > (ntp.Current_Settings.ntp_retry_interval_s * 1000)){
            ntpTimer = millis();
            Serial.print("Retrying NTP servers...");
            ClearErrorArea();
            tft.loadFont("Orbitron-Black-12");
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("Retrying NTP...", 160, 3, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
            tft.fillCircle(310, 6, 5, TFT_MAGENTA);
            if (ntp.setServer()) {
                Serial.println("Success");
                ntpErrorCleared = true;
                ClearErrorArea();
                Clock();
            } else {
                Serial.println("FAIL!");
                tft.loadFont("Orbitron-Black-12");
                tft.setTextColor(TFT_YELLOW, BG_COLOR);
                tft.drawString("NTP DISABLED", 160, 3, 2);
                tft.setTextColor(TFT_CYAN, BG_COLOR);
            }
            tft.fillCircle(310, 6, 5, BG_COLOR);
        }
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }
    else if (!ntp.Current_Settings.ntp_wifi_ok)
    {
        Serial.println("NTP is disabled. No WiFi connection available...");
        ClearErrorArea();
        tft.loadFont("Orbitron-Black-12");
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("NTP DISABLED", 160, 3, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);

        clearTextArea();
        tft.setTextColor(TFT_CYAN, BG_COLOR);
        tft.loadFont("Orbitron-Black-48");
        tft.drawCentreString(current_status, 160, 80, 2);
    }
    tft.unloadFont();
}
void Environmental(char* sys)
{
    if (sys == "Temp") {
        tft.loadFont("Orbitron-Black-24");
        if (temperature.is_error && temperature.update)
        {
            Serial.println("Drawing Temperature errors to TFT...");
            clearSensorText(1);
            clearSensorText(2);
            tft.fillTriangle(15, 215, 65, 215, 40, 180, TFT_YELLOW);   // Temperature cell
            tft.fillTriangle(95, 215, 145, 215, 120, 180, TFT_YELLOW); // Humidity cell
            tft.setTextColor(BG_COLOR, TFT_YELLOW);
            tft.drawString((String) "!", 37, 190, 2);
            tft.drawString((String) "!", 117, 190, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
        else
        {
            char buf[3];
            clearSensorText(1);
            snprintf(buf, sizeof(buf), "%04f", temperature.current_temperature);
            if (temperature.temperature_low_active)
            {
                tft.setTextColor(TFT_BLUE, BG_COLOR);
            }
            else if (temperature.temperature_warn_active)
            {
                tft.setTextColor(TFT_YELLOW, BG_COLOR);
            }
            else if (temperature.temperature_alarm_active)
            {
                tft.setTextColor(TFT_MAGENTA, BG_COLOR);
            }
            else
            {
                tft.setTextColor(TFT_CYAN, BG_COLOR);
            }
            if (buf[1] == '.')
            {
                tft.drawString((String)buf[0] + "°", 15, 190, 2);
            }
            else
            {
                tft.drawString((String)buf + "°", 15, 190, 2);
            }

            snprintf(buf, sizeof(buf), "%04f", temperature.humidity);
            if (temperature.humidity_alarm_active)
            {
                tft.setTextColor(TFT_MAGENTA, BG_COLOR);
            }
            else
            {
                tft.setTextColor(TFT_CYAN, BG_COLOR);
            }
            clearSensorText(2);
            if (buf[1] == '.')
            {
                tft.drawString((String)buf[0] + "%", 93, 190, 2);
            }
            else
            {
                tft.drawString((String)buf + "%", 90, 190, 2);
            }
            temperature.update = false;
        }
    }
    if (sys == "Air") {
        tft.loadFont("Orbitron-Black-24");
        if (airquality.is_error && airquality.update)
        {
            Serial.println("Drawing AirQuality errors to TFT...");
            clearSensorText(3);
            clearSensorText(4);
            tft.fillTriangle(175, 215, 225, 215, 200, 180, TFT_YELLOW); // TVOC cell
            tft.fillTriangle(255, 215, 305, 215, 280, 180, TFT_YELLOW); // eCO2 cell
            tft.setTextColor(BG_COLOR, TFT_YELLOW);
            tft.drawString((String) "!", 197, 190, 2);
            tft.drawString((String) "!", 277, 190, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
        else
        {
            tft.unloadFont();
            tft.setTextColor(TFT_CYAN, BG_COLOR);
            airquality.GetAirQuality(temperature.current_temperature, temperature.humidity);
            // if (temperature.Current_Settings.test_data) {
            //     airquality.GetAirQuality();
            // } else {
            //     airquality.GetAirQuality(temperature.current_temperature, temperature.humidity);
            // }
            clearSensorText(3);
            if (airquality.tvoc < 1000)
            {
                tft.loadFont("Orbitron-Black-24");
            }
            else if (airquality.tvoc > 999)
            {
                tft.loadFont("Orbitron-Black-18");
            }
            // Setup alarmed text HERE
            tft.drawString((String)airquality.tvoc, 170, 190, 2);

            clearSensorText(4);
            if (airquality.eco2 < 1000)
            {
                tft.loadFont("Orbitron-Black-24");
                tft.drawString((String)airquality.eco2, 252, 190, 2);
            }
            else if (airquality.eco2 > 999)
            {
                tft.loadFont("Orbitron-Black-18");
                tft.drawString((String)airquality.eco2, 250, 190, 2);
            }
            airquality.update = false;
        }
    }
    
    tft.unloadFont();
}
void DrawStaticSensorBoxes()
{
    // clearTextArea();
    // Environmental Section
    tft.drawFastHLine(0, 175, 320, TFT_CYAN);
    tft.drawFastVLine(80, 175, 80, TFT_CYAN);
    tft.drawFastVLine(160, 175, 80, TFT_CYAN);
    tft.drawFastVLine(240, 175, 80, TFT_CYAN);
    tft.loadFont("Orbitron-Regular-12");
    tft.setTextColor(TFT_YELLOW, BG_COLOR);
    tft.drawString((String) "Temp", 22, 220, 2);
    tft.drawString((String) "Humidity", 91, 220, 2);
    tft.drawString((String) "TVOC ppb", 165, 220, 2);
    tft.drawString((String) "eCO2 ppm", 245, 220, 2);
    tft.unloadFont();
    tft.setTextColor(TFT_CYAN, BG_COLOR);
}
void toggleTestData(char* sys){
    if (sys == "NTP") {
        ntp.Current_Settings.test_data = !ntp.Current_Settings.test_data;
        if (ntp.Current_Settings.test_data) {
            confirmUpTone();
            Serial.println("NTP test data mode ENABLED");
        } else {
            confirmDownTone();
            Serial.println("NTP test data mode DISABLED");
        }
    } else if (sys == "Temp") {
        temperature.Current_Settings.test_data = !temperature.Current_Settings.test_data;
        if (temperature.Current_Settings.test_data) {
            confirmUpTone();
            Serial.println("Temperature test data mode ENABLED");
        } else {
            confirmDownTone();
            Serial.println("Temperature test data mode DISABLED");
        }
        clearSensorText(1);
        clearSensorText(2);
        temperature.update = true;
        // Environmental("Temp");
    } else if (sys == "Air") {
        airquality.Current_Settings.test_data = !airquality.Current_Settings.test_data;
        if (airquality.Current_Settings.test_data) {
            confirmUpTone();
            airquality.update = true;
            Serial.println("Air Quality test data mode ENABLED");
        } else {
            confirmDownTone();
            Serial.println("Air Quality test data mode DISABLED");
        }
        clearSensorText(3);
        clearSensorText(4);
        airquality.update = true;
        // Environmental("Air");
    }
}
void toggleTestErrors(char* sys){
    if (sys == "NTP") {
        ntp.Current_Settings.test_error = !ntp.Current_Settings.test_error;
        if (ntp.Current_Settings.test_error) {
            confirmUpTone();
            Serial.println("NTP test error mode ENABLED");
        } else {
            confirmDownTone();
            Serial.println("NTP test error mode DISABLED");
        }
    } else if (sys == "Temp") {
        temperature.Current_Settings.test_error = !temperature.Current_Settings.test_error;
        if (temperature.Current_Settings.test_error) {
            confirmUpTone();
            Serial.println("Temeprature test error mode ENABLED");
        } else {
            confirmDownTone();
            Serial.println("Temperature test error mode DISABLED");
        }
    } else if (sys == "Air") {
        airquality.Current_Settings.test_error = !airquality.Current_Settings.test_error;
        if (airquality.Current_Settings.test_error) {
            confirmUpTone();
            Serial.println("Air Quality test error mode ENABLED");
        } else {
            confirmDownTone();
            Serial.println("Air Quality test error mode DISABLED");
        }
    }
}
void UpdateStatus()
{
    if (millis() - refreshTimer > 2000)
    {
        refreshTimer = millis();
        if (pulse)
        {
            tft.fillCircle(310, 6, 5, TFT_CYAN);
            pulse = false;
        }
        else
        {
            tft.fillCircle(310, 6, 5, BG_COLOR);
            pulse = true;
        }
        NetworkStats();
        if (is_busy)
        {
            // Only update the status text if it has changed
            if (current_status != last_status)
            {
                Serial.print("Status changed from [");
                Serial.print(last_status);
                Serial.print("] to [");
                Serial.print(current_status);
                Serial.println("]");
                clearTextArea();
                Clock();
                tft.loadFont("Orbitron-Black-32");
                tft.setTextColor(TFT_YELLOW, BG_COLOR);
                // if (ntp.error)
                // {
                //     tft.fillRect(120, 79, 200, 35, BG_COLOR);
                //     tft.drawCentreString(current_status, 220, 80, 2);
                // }
                // else
                // {
                //     tft.fillRect(120, 99, 200, 35, BG_COLOR);
                //     tft.drawCentreString(current_status, 220, 100, 2);
                // }

                tft.fillRect(120, 99, 200, 35, BG_COLOR);
                tft.drawCentreString(current_status, 220, 100, 2);
                tft.setTextColor(TFT_CYAN, BG_COLOR);
                tft.unloadFont();
                last_status = current_status;
            }
            // The flashing yellow busy circle
            if (pulse)
            {
                tft.fillCircle(50, 80, 45, TFT_YELLOW);
                tft.fillRect(25, 73, 52, 15, BG_COLOR);
            }
            else
            {
                tft.fillRect(0, 20, 130, 120, BG_COLOR);
                tft.drawCircle(50, 80, 45, TFT_YELLOW);
                tft.drawRect(25, 73, 52, 15, TFT_YELLOW);
            }
        }
        else
        {
            // Only update the status text if it has changed....
            if (current_status != last_status)
            {
                Serial.print("Status changed from [");
                Serial.print(last_status);
                Serial.print("] to [");
                Serial.print(current_status);
                Serial.println("]");
                clearTextArea();
                Clock();
                if (!ntp.error && ntp.Current_Settings.ntp_enable && ntp.Current_Settings.ntp_wifi_ok)
                {
                    tft.fillRect(0, 139, 319, 35, BG_COLOR);
                    tft.loadFont("Orbitron-Black-32");
                    tft.setTextColor(TFT_CYAN, BG_COLOR);
                    tft.drawCentreString(current_status, 160, 140, 2);
                    tft.unloadFont();
                }
                last_status = current_status;
            }
            // ...or if an NTP error just cleared, and the status needs to be redrawn in it's normal spot.
            else if (ntpErrorCleared) {
                Serial.println("Redrawing status after NTP error cleared...");
                clearTextArea();
                tft.loadFont("Orbitron-Black-32");
                tft.setTextColor(TFT_CYAN, BG_COLOR);
                tft.drawCentreString(current_status, 160, 140, 2);
                tft.unloadFont();
                ntpErrorCleared = false;
            }
        }
    }
    // Interval timing is handled by GetTemperatures() itself.
    temperature.GetTemperatures();
    if (temperature.update)
    {
        Serial.println("Measuring temp and humidity...");
        Environmental("Temp");
    }
    airquality.GetAirQuality(temperature.current_temperature, temperature.humidity);
    // Serial.print("airquality.update: "); Serial.println(airquality.update);
    if (airquality.update)
    {
        Serial.println("Measuring air quality...");
        Environmental("Air");
    }
    if (millis() - clockTimer > 60000)
    {
        clockTimer = millis();
        Serial.println("Updating time...");
        Clock();
        if (is_busy)
        {
            wiowebserver.busy_minutes -= 1;
            if (wiowebserver.busy_minutes == 0)
            {
                is_busy = false;
                current_status = "Available";
                wiowebserver.busy_minutes = 60;
            }
        }
        else
        {
            wiowebserver.busy_minutes = 60;
        }
    }
}
void CheckButtons()
{
    // Simple 500 ms debounce timer.
    if (millis() - timer > 500)
    {
        timer = millis();
        A_pressed = false;
        B_pressed = false;
        C_pressed = false;
        UP_pressed = false;
        DOWN_pressed = false;
        LEFT_pressed = false;
        RIGHT_pressed = false;
        CENTER_pressed = false;
    }
    if (A_pressed == false && digitalRead(WIO_KEY_A) == LOW)
    {
        A_pressed = true;
        if (initializing)
        {
            initializing = false;
            return;
        }
        // clearTextArea();
        if (is_busy)
        {
            is_busy = false;
            wiowebserver.busy_minutes = 60;
            current_status = "Available";
        }
        else
        {
            is_busy = true;
            current_status = "Busy";
        }
    }
    else if (B_pressed == false && digitalRead(WIO_KEY_B) == LOW)
    {
        B_pressed = true;
        if (initializing)
        {
            initializing = false;
            return;
        }
    }
    else if (C_pressed == false && digitalRead(WIO_KEY_C) == LOW)
    {
        C_pressed = true;
        if (initializing)
        {
            initializing = false;
            return;
        }
        upTone();
        // playTone(1915, 300);
        // playTone(1275, 300);
        if (current_status == "Available")
        {
            // _status_p = "Busy";
            current_status = "Busy";
        }
        else
        {
            // _status_p = "Available";
            current_status = "Available";
        }
        // wiowebserver.UpdateStatus(_status, _busy_minutes);
        // tft.loadFont("Orbitron-Black-12");
        // tft.fillRect(2, 23, 320, 40, BG_COLOR);
        // tft.setTextColor(TFT_YELLOW, BG_COLOR);
        // tft.drawString("Initializing Webserver...", 3, 23, 2);
        // tft.setTextColor(TFT_CYAN, BG_COLOR);
        // delay(2000);
        // wiowebserver.begin();

        // if (nightMode){
        //   digitalWrite(LCD_BACKLIGHT, HIGH);
        //   nightMode = false;
        // } else {
        //   digitalWrite(LCD_BACKLIGHT, LOW);
        //   nightMode = true;
        // }
    }
    if (UP_pressed == false && digitalRead(WIO_5S_UP) == LOW)
    {
        UP_pressed = true;
        if (initializing)
        {
            initializing = false;
            return;
        }
        wiowebserver.busy_minutes += 10;
        if (is_busy)
        {
            Clock();
        }
        else
        {
            tft.loadFont("Orbitron-Black-12");
            tft.drawRect(80, 135, 165, 20, TFT_CYAN);
            tft.drawString("DND minutes set to:", 82, 140, 2);
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString((String)wiowebserver.busy_minutes, 217, 140, 2);
            tft.unloadFont();
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
    }
    else if (DOWN_pressed == false && digitalRead(WIO_5S_DOWN) == LOW)
    {
        DOWN_pressed = true;
        if (initializing)
        {
            initializing = false;
            return;
        }
        if (wiowebserver.busy_minutes > 10) {
            wiowebserver.busy_minutes -= 10;
        } else {
            wiowebserver.busy_minutes  = 0;
        }
        
        if (is_busy)
        {
            Clock();
        }
        else
        {
            tft.loadFont("Orbitron-Black-12");
            tft.drawRect(80, 135, 165, 20, TFT_CYAN);
            tft.drawString("DND minutes set to:", 82, 140, 2);
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString((String)wiowebserver.busy_minutes, 217, 140, 2);
            tft.unloadFont();
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
    }
    else if (LEFT_pressed == false && digitalRead(WIO_5S_LEFT) == LOW)
    {
        if (initializing)
        {
            initializing = false;
            return;
        }
        LEFT_pressed = true;
        toggleTestData("Temp");
        toggleTestData("Air");
    }
    else if (RIGHT_pressed == false && digitalRead(WIO_5S_RIGHT) == LOW)
    {
        if (initializing)
        {
            initializing = false;
            return;
        }
        RIGHT_pressed = true;
        toggleTestErrors("NTP");
    }
    else if (CENTER_pressed == false && digitalRead(WIO_5S_PRESS) == LOW)
    {
        if (initializing)
        {
            initializing = false;
            return;
        }

        if (!muteWarning) {
            muteWarning = true;
            muteAlertTimer = millis();
            confirmUpTone();
        }
        else if (!muteAlarm) {
            muteAlarm = true;
            muteAlertTimer = millis();
            confirmUpTone();
        }
        else if (nightMode)
        {
            digitalWrite(LCD_BACKLIGHT, HIGH);
            nightMode = false;
            confirmUpTone();
        }
        else
        {
            clearTextArea();
            clearSensorText();
            ClearErrorArea();
            DrawStaticSensorBoxes();
            Clock();
            temperature.GetTemperatures();
            Environmental("Temp");
            airquality.GetAirQuality(temperature.current_temperature, temperature.humidity);
            Environmental("Air");
            UpdateStatus();
            waveTone();
        }
        CENTER_pressed = true;
    }
}
char* MinifyPage(char *page){
    char *output = "";
    for (uint16_t i=0; i<sizeof(page); i++) {
        uint16_t char_num = (uint16_t)page[i];
        if (char_num > 20 || char_num < 127) {
            output += page[i];
        }
    }
    return output;
}
void CheckAlerts() {
    // Checks for alerts every 30 seconds, and will sound them if not muted.
    if (millis() - alertTimer > 30000)
    {
        // Alerts will stay muted for 1 hour
        if (muteWarning && millis() - muteAlertTimer > 3600000) {
            Serial.println("Unmuting warning...");
            muteWarning = false;
        }
        if (muteAlarm && millis() - muteAlertTimer > 3600000) {
            Serial.println("Unmuting alarm...");
            muteAlarm = false;
        }

        if (temperature.temperature_warn_active && !muteWarning)
        {
            Serial.println("WARNING ACTIVE!");
            warnTone();
        }
        else if (temperature.temperature_alarm_active && !muteAlarm)
        {
            Serial.println("ALARM ACTIVE!");
            alarmTone();
        }
        alertTimer = millis();
    }
}
uint8_t init_SD()
{
    tft.setTextColor(TFT_WHITE, BG_COLOR);
    int boot_yDraw = 2 + TEXT_HEIGHT;
    tft.drawString("Mounting SD...", 3, boot_yDraw, 2);
    initTimer = millis();
    bool sd_success = true;
    while (!SD.begin(SDCARD_SS_PIN, SDCARD_SPI))
    {
        if (millis() - initTimer > 10000)
        {
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("FAIL!", 98, boot_yDraw, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
            sd_success = false;
            break;
        }
    }
    if (sd_success)
    {
        Serial.println("SD successfully mounted...");
        tft.drawString("SUCCESS", 98, boot_yDraw, 2);
    }
    delay(2000);
    if (sd_success && SD.exists(secretsFileName))
    {
        tft.drawString("Loading config.cfg...", 3, boot_yDraw += TEXT_HEIGHT, 2);
        if (readConfiguration())
        {
            Serial.println("config.cfg load SUCCESS");
            tft.drawString("SUCCESS", 140, boot_yDraw, 2);
        }
        else
        {
            Serial.println("config.cfg load FAIL!");
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("FAIL!", 140, boot_yDraw, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
            sd_success = false;
        }
    }
    else
    {
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("FAIL!", 140, boot_yDraw, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
        sd_success = false;
        delay(1000);
    }
    delay(2000);
    tft.fillScreen(BG_COLOR);
    tft.setTextColor(TFT_WHITE, BG_COLOR);
    return sd_success;
}
void setup()
{
    tft.init();
    tft.setRotation(3);
    digitalWrite(LCD_BACKLIGHT, HIGH);
    tft.fillScreen(BG_COLOR);
    tft.setFreeFont(FM9);
    tft.drawString("System BOOT", 3, 2, 2);
    bool sd_success = init_SD();
    tft.setTextColor(TFT_WHITE, BG_COLOR);
    if (!sd_success)
    {
        bootup_title_text = "S T A T U S  T E R M I N A L";
    }

    tft.drawFastHLine(3, 8, 314, TFT_WHITE);
    tft.drawString(bootup_title_text, 68, yDraw, 2);
    tft.setTextColor(TFT_CYAN, BG_COLOR);
    tft.drawString("Initializing top buttons...", 3, yDraw+=TEXT_HEIGHT, 2);
    // Built in buttons
    pinMode(WIO_KEY_A, INPUT_PULLUP);
    pinMode(WIO_KEY_B, INPUT_PULLUP);
    pinMode(WIO_KEY_C, INPUT_PULLUP);

    tft.drawString("Initializing 5-way switch...", 3, yDraw+=TEXT_HEIGHT, 2);
    // 5 Way Switch
    pinMode(WIO_5S_UP, INPUT_PULLUP);
    pinMode(WIO_5S_DOWN, INPUT_PULLUP);
    pinMode(WIO_5S_LEFT, INPUT_PULLUP);
    pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
    pinMode(WIO_5S_PRESS, INPUT_PULLUP);

    tft.drawString("Initializing buzzer...", 3, yDraw+=TEXT_HEIGHT, 2);
    // Buzzer
    pinMode(WIO_BUZZER, OUTPUT);
    if (!sd_success)
    {
        tft.drawString("No SD Card. Booting with default config...", 3, yDraw += TEXT_HEIGHT, 2);
    }
    if (temperature.Current_Settings.enable_temperature)
    {
        Serial.println("Initializing DHT-22 temperature sensor...");
        tft.drawString("Starting DHT-22 temp sensor...", 3, yDraw += TEXT_HEIGHT, 2); // pin BCM23
        // Init temperatures
        if (temperature.begin())
        {
            tft.drawString("SUCCESS", 203, yDraw, 2);
        }
        else
        {
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("FAIL!", 203, yDraw, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
    }

    // Init AirQuality
    if (airquality.Current_Settings.enable_air_quality)
    {
        Serial.println("Initializing SGP30 gas sensor...");
        tft.drawString("Starting SGP30 gas sensor...", 3, yDraw += TEXT_HEIGHT, 2);
        if (airquality.begin())
        {
            tft.drawString("SUCCESS", 189, yDraw, 2);
        }
        else
        {
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("FAIL!", 189, yDraw, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
    }

    // Init WiFi
    if (network.Current_Settings.wifi_enable)
    {
        Serial.println("Connecting to WiFi...");
        tft.drawString("Connecting wifi. Please wait...", 3, yDraw += TEXT_HEIGHT, 2);
        NetworkStats();
        if (network.isConnected) {
            Serial.print("Wifi connected. IP address is: ");
            Serial.println(network.IP);
        }
        else
        {
            Serial.println("Wifi connection FAILED!");
        }

    }
    else
    {
        Serial.print("WiFi is DISABLED");
        ntp.Current_Settings.ntp_wifi_ok = false;
        wiowebserver.Current_Settings.webserv_wifi_ok = false;
        tft.drawString("WiFi is", 3, yDraw += TEXT_HEIGHT, 2);
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("DISABLED", 52, yDraw, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }

    // Init NTP
    if (!ntp.Current_Settings.ntp_enable)
    {
        tft.drawString("NTP is", 3, yDraw += TEXT_HEIGHT, 2);
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("DISABLED", 50, yDraw, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }
    else if (network.isConnected)
    {
        tft.drawString("Connecting to NTP server...", 3, yDraw += TEXT_HEIGHT, 2);
        if (ntp.init())
        {
            tft.drawString("SUCCESS", 185, yDraw, 2);
        }
        else
        {
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("FAIL!", 185, yDraw, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
    } else {
        tft.drawString("Connecting to NTP server...", 3, yDraw += TEXT_HEIGHT, 2);
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("NO WIFI!", 185, yDraw, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }

    if (!wiowebserver.Current_Settings.webserv_enable)
    {
        tft.drawString("Webserver is", 3, yDraw += TEXT_HEIGHT, 2);
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("DISABLED", 90, yDraw, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }
    else if (network.isConnected)
    {
        // We send the webserver the temperature and airquality objects, as well
        //  the variables we use to track the current status of the user and the
        //  amount of minutes busy. Share and share alike!

        wiowebserver = WioWebServer(temperature, airquality, current_status, busy_minutes, is_busy, network.IP);
        wiowebserver.Current_Settings = webserver_settings;
        if (wiowebserver.Current_Settings.webserv_enable && wiowebserver.Current_Settings.webserv_use_custom) {
            // wiowebserver.UseTestPage();
            Serial.print("Loading custom webserver index page...");
            tft.drawString("Loading custom webpage...", 3, yDraw += TEXT_HEIGHT, 2);
            if (SD.exists("/index.html"))
            {
                File htmlPage = SD.open("/index.html", FILE_READ);
                String htmlString;
                uint16_t last_char_num = 0;
                while (htmlPage.available()) {
                    uint16_t char_num = htmlPage.read();
                    // We clean up the HTML a bit by removing as much whitespace as possible.
                    if (char_num > 31 || char_num < 127) {
                        if (char_num == 32) {
                            if (last_char_num != 32){
                                wiowebserver.Current_Settings.index_page += (char)char_num;
                            }
                        } else if (char_num < 10 || char_num > 13) {
                            wiowebserver.Current_Settings.index_page += (char)char_num;
                        }
                        last_char_num = char_num;
                    }
                }
                htmlPage.close();
                Serial.println("SUCCESS");
                tft.drawString("SUCCESS", 170, yDraw, 2);
            } else {
                Serial.println("FAIL!");
                Serial.println("Could not find index.html on the SD card!");
                tft.setTextColor(TFT_YELLOW, BG_COLOR);
                tft.drawString("FAIL!", 170, yDraw, 2);
                tft.setTextColor(TFT_CYAN, BG_COLOR);
                wiowebserver.Current_Settings.webserv_use_custom = false;
            }
        }
        tft.drawString("Initializing webserver...", 3, yDraw += TEXT_HEIGHT, 2);
        if (wiowebserver.begin())
        {
            tft.drawString("SUCCESS", 155, yDraw, 2);
        }
        else
        {
            tft.setTextColor(TFT_YELLOW, BG_COLOR);
            tft.drawString("FAIL!", 155, yDraw, 2);
            tft.setTextColor(TFT_CYAN, BG_COLOR);
        }
    }
    else
    {
        tft.drawString("Initializing webserver...", 3, yDraw += TEXT_HEIGHT, 2);
        tft.setTextColor(TFT_YELLOW, BG_COLOR);
        tft.drawString("NO WIFI!", 155, yDraw, 2);
        tft.setTextColor(TFT_CYAN, BG_COLOR);
    }

    // wiomqtt = WioMQTT();
    Serial.println("Initialization complete.");
    tft.drawString("Setup complete.", 3, yDraw += TEXT_HEIGHT, 2);
    tft.setTextColor(TFT_WHITE, BG_COLOR);
    tft.drawString("Press any button to continue...", 3, 221, 2);
    tft.setTextColor(TFT_CYAN, BG_COLOR);

    initTimer = millis();
    initializing = true;
    while (initializing)
    {
        CheckButtons();
        if (sd_success) {
            if (millis() - initTimer > 20000)
            {
                break;
            }
        }
    }
    Serial.println("Starting main loop...");
    current_status = "Available";
    tft.fillScreen(BG_COLOR);

    DrawStaticSensorBoxes();

    initializing = false;
    NetworkStats();
    temperature.update = true;
    airquality.update = true;
    UpdateStatus();
}
void loop()
{
    UpdateStatus();
    CheckButtons();
    if (wiowebserver.Current_Settings.webserv_enable)
    {
        wiowebserver.Serve();
    }
    CheckAlerts();
    CheckButtons();
}
