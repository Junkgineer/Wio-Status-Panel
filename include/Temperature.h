// All of the values in struct Settings{} will be replaced those in config.cfg
// (if they exist). The current values are defaults.
#ifndef Temperature_h
#define Temperature_h
#include <stdint.h>

class Temperature {
  public:
    Temperature();
    int8_t GetTemperatures();
    void CheckTempAlarms();
    void CheckHumidityAlarms();
    int8_t begin();
    // uint32_t refresh_rate;
    bool is_error;
    double current_temperature;
    double current_temperature_last;
    double current_heat_index;
    double last_heat_index;
    double high_temperature;
    bool temperature_low_active;
    bool temperature_warn_active;
    bool temperature_alarm_active;
    bool heat_index_alarm_active;
    double humidity;
    double humidity_last;
    double humidity_alarm;
    bool humidity_low_alarm_active;
    bool humidity_warn_alarm_active;
    bool humidity_alarm_active;
    bool update;
    
    struct Settings {
      bool enable_temperature     = true;
      bool is_fahrenheit          = true;
      uint16_t read_interval_s    = 5000;
      bool enable_temp_alarms     = true;
      int heat_index_alarm        = 95;
      int high_temp_alarm         = 90;
      int warn_temp_alarm         = 80;
      int low_temp_alarm          = 40;
      bool enable_humidity_alarms = true;
      int high_humidity_alarm     = 60;
      int warn_humidity_alarm     = 50;
      int low_humidity_alarm      = 10;
      bool test_error             = false;
      bool test_data              = false;
      double test_temperature     = 75;
      double test_humidity        = 38;
    } Current_Settings;
};
#endif
