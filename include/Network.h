#ifndef Network_h
#define Network_h
// #ifndef HTTPClient_h
// #define HTTPClient_h
// #endif
// #ifndef ArduinoJson_h
// #define ArduinoJson_h
// #endif
#include <rpcWiFi.h>
#include <HTTPClient.h>

class Network
{
public:
    struct Settings
    {
        bool wifi_enable = true;
        char *wifi_ssid;
        char *wifi_pass;
        bool use_advanced = false;
        IPAddress static_ip;
        IPAddress gateway_ip;
        IPAddress subnet_ip;
        IPAddress primary_dns_ip;
        IPAddress secondary_dns_ip;
        bool wifi_retry = true;
        uint16_t wifi_retry_interval_s = 600;
    } Current_Settings;
    Network();
    Network(char *w_ssid);
    Network(char *w_ssid, char *w_password);
    Network(char w_ssid, char w_password);
    Network(Settings settings);
    void init();
    int8_t ConnectWifi();
    int8_t ConnectWifi(char *w_ssid, char *w_password);
    int8_t Retry();
    int8_t DisconnectWifi();
    void PullAPIData();
    IPAddress IP;
    bool isConnected;
    bool update;
    char *ssid;
    char *password;
    uint8_t coreTemp;

private:
    bool _initialized = false;
};
#endif

