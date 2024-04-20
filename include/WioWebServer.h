// All of the values in struct Settings{} will be replaced those in config.cfg
// (if they exist). The current values are defaults.
#ifndef WioWebServer_h
#define WioWebServer_h
#include <stdint.h>
#include "Arduino.h"
//  #include <HTTPClient.h>
// #include <QNEthernet.h>
#include <WebServer.h>
#include "Temperature.h"
#include "AirQuality.h"

class WioWebServer {
  private:
    Temperature *_temperature;
    AirQuality *_airquality;
    String *_current_status;
    uint8_t *_busy_minutes;
    bool *_is_busy;
    IPAddress *_server_IP;
  public:
    WioWebServer();
    WioWebServer(Temperature &w_temp);
    WioWebServer(Temperature &w_temp, AirQuality &w_air, String &w_status, uint8_t &w_busy_minutes, bool &w_is_busy, IPAddress &ip_address);
    static void UpdateStatus(String *status, uint8_t *busy_mins);
    static void SetMinutes(uint8_t *busy_mins);
    uint8_t begin();
    void Serve();
    void GetExample();
    void UseTestPage();
    IPAddress ServerIP;
    bool is_busy;
    bool is_ready;
    // String *current_status;
    uint16_t busy_minutes = 60;
    struct Settings {
      bool webserv_enable = true;
      bool webserv_use_custom = true;
      bool webserv_wifi_ok = true;
      String index_page;
      String test_page;
    } Current_Settings;
};
#endif
