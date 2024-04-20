#include "NTP.h"
#include <time.h>
WiFiUDP ntpUDP;
NTPClient TimeClient(ntpUDP, -18000);   // This is supposed to be replaced by the value in config.cfg, but it's currently not

NTP::NTP(){
    
}

uint8_t NTP::init() {
    char msg_buffer[100];
    if (!NTP::setServer()) {
        NTP::Current_Time = "INIT ERROR";
        Serial.println("NTP ERROR. Could not connect to any of the NTP servers. NTP has been DISABLED.");
        if (NTP::Current_Settings.ntp_retry) {
            snprintf(msg_buffer, sizeof(msg_buffer), "Will retry in %d seconds. [CFG: ntp_retry=true, ntp_retry_interval_s=%d]", NTP::Current_Settings.ntp_retry_interval_s,NTP::Current_Settings.ntp_retry_interval_s);
            Serial.println(msg_buffer);
        } else {
            Serial.println("Connection retries will not be attempted. [CFG: ntp_retry=false]");
        }
        return 0;
    }
    return 1;
}
uint8_t NTP::setServer() {
    char* timeservers[3] = {NTP::Current_Settings.ntp_server1, NTP::Current_Settings.ntp_server2, NTP::Current_Settings.ntp_server3};
    uint32_t ntpInitTimer = millis();
    char msg_buffer[100];
    TimeClient.end();
    for (int i = 0; i < 3; i++) {
        TimeClient.setPoolServerName(timeservers[i]);
        TimeClient.begin();
        TimeClient.getFormattedTime();
        snprintf(msg_buffer, sizeof(msg_buffer), "Trying NTP server option %d: %s...", i+1, timeservers[i]);
        Serial.print(msg_buffer);
        while (!TimeClient.forceUpdate()){
            if(millis() - ntpInitTimer > 5000) {
                Serial.println("FAIL!");
                this->error = true;
                ntpInitTimer = millis();
                break;
            }
        }
        if (TimeClient.forceUpdate()) {
            Serial.println("SUCCESS!");
            snprintf(msg_buffer, sizeof(msg_buffer), "Syncing NTP time with %s", timeservers[i]);
            Serial.println(msg_buffer);
            NTP::Current_Time = TimeClient.getFormattedTime();
            this->error = false;
            return 1;
        }
    }
    return 0;
}
uint8_t NTP::getTime() {
    if (Current_Settings.test_data) {
        NTP::Current_Time = Current_Settings.test_time;
        Serial.println("Using test data for current time...");
    } else if (Current_Settings.test_error) {
        this->error = true;
        NTP::Current_Time = "UPDATE ERROR";
        Serial.println("SIMULATED NTP Update Error!");
        return 0;
    } else {
        // I'm using a timer in main(), so we'll use forceUpdate() here instead of update()
        if (TimeClient.forceUpdate()) {
            this->error = false;
            NTP::Current_Time = TimeClient.getFormattedTime();
            return 1;
        } else {
            this->error = true;
            NTP::Current_Time = "UPDATE ERROR";
            Serial.println("NTP Update Error!");
            return 0;
        }
    }
}

