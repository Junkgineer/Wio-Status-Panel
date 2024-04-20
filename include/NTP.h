// All of the values in struct Settings{} will be replaced those in config.cfg
// (if they exist). The current values are defaults.
#ifndef NTP_h
#define NTP_h
#include <stdint.h>
#include <Arduino.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

class NTP {
  public:
    NTP();
    uint8_t init();
    uint8_t getTime();
    uint8_t setServer();
    String Current_Time;
    bool error;
    struct Settings {
      bool ntp_enable               = true;
      bool ntp_retry                = true;
      bool ntp_wifi_ok              = true;
      uint16_t timezone_offset_s    = 0;
      uint16_t ntp_retry_interval_s = 600;
      char* ntp_server1             = "ntp-b.nist.gov";
      char* ntp_server2             = "192.168.0.1";
      char* ntp_server3             = "127.0.0.1";
      bool test_error               = false;
      bool test_data                = false;
      String test_time              = "10:32:41";
    } Current_Settings;
};
#endif