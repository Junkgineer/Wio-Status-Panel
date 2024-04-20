#include "Temperature.h"
#include <DHT.h>

#define DHTPIN BCM23
#define DHTTYPE DHT22

DHT dht(DHTPIN, DHTTYPE);
int temperatureTimer;

Temperature::Temperature()
{
}
int8_t Temperature::begin()
{
    Temperature::is_error = false;
    temperatureTimer = millis();
    dht.begin();
    delay(2000);
    Temperature::humidity = dht.readHumidity();
    Temperature::current_temperature = dht.readTemperature(Temperature::Current_Settings.is_fahrenheit);

    if (isnan(Temperature::humidity) || isnan(Temperature::current_temperature))
    {
        if (Current_Settings.test_data) {
            Temperature::humidity = Current_Settings.test_humidity;
            Temperature::current_temperature = Current_Settings.test_temperature;
            Temperature::is_error = false;
            return 1;
        }
        Serial.println("Temp sensor error! Could not initialize temperature sensor.");
        // Temperature::is_error = true;
        return 0;
    }
    else
    {
        return 1;
    }
}
int8_t Temperature::GetTemperatures()
{
    if (millis() - temperatureTimer > (Temperature::Current_Settings.read_interval_s * 1000))
    {
        if (Current_Settings.test_data) {
            if (Temperature::current_temperature == 0 || isnan(Temperature::current_temperature)) {;
                Temperature::current_temperature = Current_Settings.test_temperature;
            }
            if (Temperature::humidity == 0 || isnan(Temperature::humidity)) {
                Temperature::humidity = Current_Settings.test_humidity;
            }
            Temperature::is_error = false;
            long rand = random(30);
            if (rand >= 10 && rand < 20) {
                Temperature::humidity_last = Temperature::humidity;
                Temperature::current_temperature_last = Temperature::current_temperature;
                Temperature::humidity = Temperature::humidity += random(3);
                Temperature::current_temperature = Temperature::current_temperature += random(3);
                Temperature::update = true;
                Serial.println("Updating test temp and humidity measurements...");
            } else if (rand >= 20 && rand < 30) {
                Temperature::humidity_last = Temperature::humidity;
                Temperature::current_temperature_last = Temperature::current_temperature;
                Temperature::humidity = Temperature::humidity -= random(3);
                Temperature::current_temperature = Temperature::current_temperature -= random(3);
            }
            temperatureTimer = millis();
            return 1;
        } else if (Current_Settings.test_error)
        {
            Serial.println("SIMULATED Temp sensor error! Could not read from temperature sensor.");
            Temperature::is_error = true;
            temperatureTimer = millis();
            return 0;
        }
        Temperature::humidity = dht.readHumidity();
        Temperature::current_temperature = dht.readTemperature(Temperature::Current_Settings.is_fahrenheit);

        if (isnan(Temperature::humidity) || isnan(Temperature::current_temperature))
        {
            // Serial.println("Temperature or Humidity reading is NaN!");
            if (!Temperature::is_error)
            {
                Serial.println("Temp sensor error! Could not read from temperature sensor.");
                Temperature::update = true;
                Temperature::is_error = true;
            } else if(Temperature::is_error && Temperature::update) {
                Temperature::update = false;
            }
            return 0;
        }
        else
        {
            if (Temperature::is_error)
            {
                Serial.println("Temp sensor back online.");
                Temperature::is_error = false;
            }
            Temperature::current_heat_index = dht.computeHeatIndex(Temperature::current_temperature, Temperature::humidity, Temperature::Current_Settings.is_fahrenheit);

            if (Temperature::current_temperature > Temperature::high_temperature)
            {
                Temperature::high_temperature = Temperature::current_temperature;
            }

            // In order to only refresh the screen when we have to,
            //  we don't redraw anything unless something has changed.
            Temperature::update = false;
            if (Temperature::humidity != Temperature::humidity_last)
            {
                Temperature::humidity_last = Temperature::humidity;
                Temperature::update = true;
            }
            if (Temperature::current_temperature != Temperature::current_temperature_last)
            {
                Temperature::current_temperature_last = Temperature::current_temperature;
                Temperature::update = true;
            }
            if (Temperature::current_heat_index != Temperature::last_heat_index)
            {
                Temperature::last_heat_index = Temperature::current_heat_index;
                Temperature::update = true;
            }
        }

        temperatureTimer = millis();
        if (Temperature::Current_Settings.enable_temp_alarms)
        {
            Temperature::CheckTempAlarms();
        }
        else
        {
            Temperature::temperature_low_active = false;
            Temperature::temperature_warn_active = false;
            Temperature::temperature_alarm_active = false;
        }
        if (Temperature::Current_Settings.enable_humidity_alarms)
        {
            Temperature::CheckHumidityAlarms();
        }
        else
        {
            Temperature::humidity_low_alarm_active = false;
            Temperature::humidity_warn_alarm_active = false;
            Temperature::humidity_alarm_active = false;
        }
        return 1;
    }
    return 1;
}
void Temperature::CheckTempAlarms()
{
    if (Temperature::Current_Settings.low_temp_alarm != 0)
    {
        // LOW Temperature Alarm
        if (Temperature::current_temperature < Temperature::Current_Settings.low_temp_alarm)
        {
            Temperature::temperature_low_active = true;
            Temperature::temperature_warn_active = false;
            Temperature::temperature_alarm_active = false;
        }
    }
    // NO Temperature Alarm
    if (Temperature::current_temperature > Temperature::Current_Settings.low_temp_alarm && Temperature::current_temperature < Temperature::Current_Settings.warn_temp_alarm)
    {
        Temperature::temperature_low_active = false;
        Temperature::temperature_warn_active = false;
        Temperature::temperature_alarm_active = false;
    }
    if (Temperature::Current_Settings.warn_temp_alarm != 0)
    {
        // WARNING Temperature Alarm
        if (Temperature::current_temperature > Temperature::Current_Settings.warn_temp_alarm && Temperature::current_temperature < Temperature::Current_Settings.high_temp_alarm)
        {
            Temperature::temperature_low_active = false;
            Temperature::temperature_warn_active = true;
            Temperature::temperature_alarm_active = false;
        }
    }
    if (Temperature::Current_Settings.high_temp_alarm != 0)
    {
        // HIGH Temperature Alarm
        if (Temperature::current_temperature > Temperature::Current_Settings.high_temp_alarm)
        {
            Temperature::temperature_low_active = false;
            Temperature::temperature_warn_active = false;
            Temperature::temperature_alarm_active = true;
        }
    }
    // HIGH Heat Index Alarm
    if (Temperature::current_heat_index > Temperature::Current_Settings.heat_index_alarm)
    {
        Temperature::heat_index_alarm_active = true;
    }
    else
    {
        Temperature::heat_index_alarm_active = false;
    }
}
void Temperature::CheckHumidityAlarms()
{
    if (Temperature::Current_Settings.low_humidity_alarm != 0)
    {
        // LOW Humidity Alarm
        if (Temperature::humidity < Temperature::Current_Settings.low_humidity_alarm)
        {
            Temperature::humidity_low_alarm_active = true;
            Temperature::humidity_warn_alarm_active = false;
            Temperature::humidity_alarm_active = false;
        }
    }
    // NO Humidity Alarm
    if (Temperature::humidity > Temperature::Current_Settings.low_humidity_alarm && Temperature::humidity < Temperature::Current_Settings.warn_humidity_alarm)
    {
        Temperature::humidity_low_alarm_active = false;
        Temperature::humidity_warn_alarm_active = false;
        Temperature::humidity_alarm_active = false;
    }
    if (Temperature::Current_Settings.warn_humidity_alarm != 0)
    {
        // WARN Humidity Alarm
        if (Temperature::humidity > Temperature::Current_Settings.warn_humidity_alarm && Temperature::humidity < Temperature::Current_Settings.high_humidity_alarm)
        {
            Temperature::humidity_low_alarm_active = false;
            Temperature::humidity_warn_alarm_active = true;
            Temperature::humidity_alarm_active = false;
        }
    }
    if (Temperature::Current_Settings.high_humidity_alarm != 0)
    {
        // HIGH Humidity Alarm
        if (Temperature::humidity > Temperature::Current_Settings.high_humidity_alarm)
        {
            Temperature::humidity_low_alarm_active = false;
            Temperature::humidity_warn_alarm_active = false;
            Temperature::humidity_alarm_active = true;
        }
    }
}