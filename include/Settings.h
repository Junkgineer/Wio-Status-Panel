#ifndef Settings_h
#define Settings_h
#include <stdint.h>
#include <Arduino.h>
#include <TFT_eSPI.h>

class Settings {
  public:
    Settings();
    void init();
    void CheckButtons();
    void Selection();
    void ReadSettingsFile();
    bool KeyboardOn;
    bool WifiOn;
    bool SerialOn;
    bool WebserverOn;
    bool SDSuccess;
    char* WiFiSSID;
    char* WiFiPassword;
    String IOSerialPort;
    uint16_t SerialBaudRate;
    struct SettingsOptions {
      bool keyboard_enabled;
      String IO_serial_port;
      bool wifi_enabled;
      String wifi_ssid;
      String wifi_password;
      bool wifi_restart_on_error;
      String serial_user;
      String serial_pass;
      bool webserver_enabled;
      bool use_remote_config;
      String remote_config_uri;
      bool alarms_on;
      int high_temp_warn;
      int high_temp_alarm;
      int high_humid_warn;
      int high_humid_alarm;
      bool shutdown_on_alarm;
    };
    struct ScreenItem {
      int Number;
      String Field;
      String Value;
      int xPos;
      int yPos;
      bool hasValue;
      int forScreen;
      String Options[5];
    };    
    struct Screen {
      int Number;
      String Name;
      int Max_Items;
      int Selected_Item;
      int Selected_Index;
      int Menu_Start_X;
      int Menu_Start_Y;
      int Menu_Item_Height;
      struct ScreenItem Items[5];
    };
    SettingsOptions ActiveSettings;
    Screen MainScreen;
    Screen SerialOptions;
    Screen WifiOptions;
    Screen GeneralOptions;

//    SettingsOptions SettingsOptions;
  private:
    
};
#endif
