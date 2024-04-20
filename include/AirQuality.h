#ifndef AirQuality_h
#define AirQuality_h
#include <stdint.h>

class AirQuality
{
public:
    AirQuality();
    uint8_t GetAirQuality();
    uint8_t GetAirQuality(float, float);
    void CheckAlarms();
    uint8_t begin();
    uint16_t refresh_rate;
    bool is_error;
    uint16_t tvoc;
    uint16_t tvoc_last;
    uint16_t eco2;
    uint16_t eco2_last;
    uint16_t eco2_high;
    uint16_t tvoc_high;
    uint16_t rawH2;
    uint16_t rawH2_high;
    uint16_t rawEthanol;
    uint16_t rawEthanol_high;
    uint16_t TVOC_base, eCO2_base;
    bool eco2_warn_active;
    bool eco2_alarm_active;
    bool tvoc_warn_active;
    bool tvoc_alarm_active;
    bool alarms_on;
    bool update;
    struct Settings
    {
        bool enable_air_quality = true;
        uint16_t air_quality_interval_s = 2;
        bool enable_tvoc_alarm = true;
        uint16_t high_tvoc_alarm = 2000;
        uint16_t warning_tvoc_alarm = 1000;
        uint16_t low_tvoc_alarm = 0;
        bool enable_eco2_alarm = true;
        uint16_t high_eco2_alarm = 10000;
        uint16_t warning_eco2_alarm = 5000;
        uint16_t low_eco2_alarm = 0;
        bool test_error = false;
        bool test_data = false;
        uint16_t test_TVOC = 900;
        uint16_t test_eCO2 = 1568;
        uint16_t test_rawH2 = 0;
        uint16_t test_rawEthanol = 0;
    } Current_Settings;

private:
    void dumpToConsole();
};
#endif
