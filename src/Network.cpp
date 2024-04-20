#include "Network.h"

Network::Network()
{
}
Network::Network(char *w_ssid)
{
}
Network::Network(char *w_ssid, char *w_password)
{
    Network::Current_Settings.wifi_ssid = w_ssid;
    Network::Current_Settings.wifi_pass = w_password;
}
Network::Network(Settings settings)
{
    Network::Current_Settings = settings;
}
void Network::init()
{
    WiFi.mode(WIFI_STA);
    Network::isConnected = false;
}
int8_t Network::ConnectWifi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        Network::IP = WiFi.localIP();
        Network::isConnected = true;
        return 1;
    }
    else
    {
        if (Network::Current_Settings.use_advanced){
            Serial.println("Using advanced IP settings:");
            Serial.print("  Static IP: ");
            Serial.println(Network::Current_Settings.static_ip);
            Serial.print("  Gateway IP: ");
            Serial.println(Network::Current_Settings.gateway_ip);
            Serial.print("  Subnet: ");
            Serial.println(Network::Current_Settings.subnet_ip);
            Serial.print("  Primary DNS: ");
            Serial.println(Network::Current_Settings.primary_dns_ip);
            Serial.print("  Secondary DNS: ");
            Serial.println(Network::Current_Settings.secondary_dns_ip);
            WiFi.config(Network::Current_Settings.static_ip, Network::Current_Settings.gateway_ip,Network::Current_Settings.subnet_ip, Network::Current_Settings.primary_dns_ip, Network::Current_Settings.secondary_dns_ip);
        }
        WiFi.begin(Network::Current_Settings.wifi_ssid, Network::Current_Settings.wifi_pass);
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            Serial.println("Could not connect to Wifi!!");
            Network::isConnected = false;
            Serial.print(Network::Current_Settings.wifi_ssid);
            Serial.print(" | ");
            Serial.println(Network::Current_Settings.wifi_pass);
            return 0;
        }
        else
        {
            Network::IP = WiFi.localIP();
            Network::isConnected = true;
            return 1;
        }
    }
}
int8_t Network::ConnectWifi(char *w_ssid, char *w_password)
{
    Network::Current_Settings.wifi_ssid = w_ssid;
    Network::Current_Settings.wifi_pass = w_password;
    int8_t _success = 0;
    if (WiFi.status() == WL_CONNECTED)
    {
        Network::IP = WiFi.localIP();
        Network::isConnected = true;
        if (!Network::_initialized) {
            Network::update = true;
        }
        _success = 1;
    }
    else
    {
        if (Network::Current_Settings.use_advanced){
            Serial.println("Using advanced IP settings:");
            Serial.print("  Static IP: ");
            Serial.println(Network::Current_Settings.static_ip);
            Serial.print("  Gateway IP: ");
            Serial.println(Network::Current_Settings.gateway_ip);
            Serial.print("  Subnet: ");
            Serial.println(Network::Current_Settings.subnet_ip);
            Serial.print("  Primary DNS: ");
            Serial.println(Network::Current_Settings.primary_dns_ip);
            Serial.print("  Secondary DNS: ");
            Serial.println(Network::Current_Settings.secondary_dns_ip);
            WiFi.config(Network::Current_Settings.static_ip, Network::Current_Settings.gateway_ip,Network::Current_Settings.subnet_ip, Network::Current_Settings.primary_dns_ip, Network::Current_Settings.secondary_dns_ip);
        }
        WiFi.begin(Network::Current_Settings.wifi_ssid, Network::Current_Settings.wifi_pass);
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            Serial.println("Could not connect to Wifi!!");
            if (Network::isConnected) {
                Network::update = true;
            }
            Network::isConnected = false;
            _success = 0;
        }
        else
        {
            Network::IP = WiFi.localIP();
            if (Network::isConnected) {
                Network::update = false;
            }
            Network::isConnected = true;
            _success = 1;
        }
    }
    if (!Network::_initialized) {
        Network::update = true;
        Network::_initialized = true;
    }
    return _success;
}
int8_t Network::Retry()
{
    int8_t _success = 0;
    if (WiFi.status() == WL_CONNECTED)
    {
        Network::IP = WiFi.localIP();
        Network::isConnected = true;
        _success = 1;
    }
    else if (!Network::Current_Settings.wifi_ssid || !Network::Current_Settings.wifi_pass) {
        Serial.println("Missing SSID or password!");
        _success = 0;
    }
    else
    {
        WiFi.begin(Network::Current_Settings.wifi_ssid, Network::Current_Settings.wifi_pass);
        if (WiFi.waitForConnectResult() != WL_CONNECTED)
        {
            Serial.println("Could not connect to Wifi!!");
            if (Network::isConnected) {
                Network::update = true;
            }
            Network::isConnected = false;
            _success = 0;
        }
        else
        {
            Network::IP = WiFi.localIP();
            if (Network::isConnected) {
                Network::update = false;
            }
            Network::isConnected = true;
            _success = 1;
        }
    }
    return _success;
}
int8_t Network::DisconnectWifi()
{
    WiFi.disconnect();
    delay(500);
    if (WiFi.status() == WL_DISCONNECTED)
    {
        Network::isConnected = false;
        return 1;
    }
    else
    {
        return 0;
    }
}
void Network::PullAPIData()
{
}
