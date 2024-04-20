#include "AirQuality.h"
#include "Adafruit_SGP30.h"
Adafruit_SGP30 sgp;

float temperature_last;
float humidity_last;
uint32_t airTimer;

uint32_t getAbsoluteHumidity(float t, float h)
{
    // approximation formula from Sensirion SGP30 Driver Integration chapter 3.15
    const float absoluteHumidity = 216.7f * ((h / 100.0f) * 6.112f * exp((17.62f * t) / (243.12f + t)) / (273.15f + t)); // [g/m^3]
    const uint32_t absoluteHumidityScaled = static_cast<uint32_t>(1000.0f * absoluteHumidity);                           // [mg/m^3]
    return absoluteHumidityScaled;
}

AirQuality::AirQuality()
{
}

uint8_t AirQuality::begin()
{
    AirQuality::is_error = false;
    AirQuality::alarms_on = true;
    airTimer = millis();
    AirQuality::refresh_rate = 2000;
    bool res = sgp.begin();
    delay(1000);
    if (Current_Settings.test_error)
    {
        Serial.println("SIMULATED Gas sensor error! Could not initialize air quality sensor.");
        AirQuality::is_error = true;
        return 0;
    }
    else if (Current_Settings.test_data)
    {
        Serial.println("Initializing sgp30 with TEST data...");
        AirQuality::eco2 = Current_Settings.test_eCO2;
        AirQuality::tvoc = Current_Settings.test_TVOC;
        AirQuality::rawH2 = Current_Settings.test_rawH2;
        AirQuality::rawEthanol = Current_Settings.test_rawEthanol;
        AirQuality::is_error = false;
        return 1;
    }
    else if (!res)
    {
        Serial.println("Gas sensor error! Could not initialize air quality sensor.");
        AirQuality::is_error = true;
        return 0;
    } else {
        return 1;
    }
    // sgp.setIAQBaseline(0x8E2A, 0x925D); //TODO: Recalibrate baseline
    
}

uint8_t AirQuality::GetAirQuality()
{
    if (millis() - airTimer > AirQuality::refresh_rate)
    {
        AirQuality::update = false;
        if (Current_Settings.test_data)
        {
            Serial.println("Updating test air quality measurements...");
            AirQuality::is_error = false;

            // Generate some random changes in the data.
            // 33% chance of adding a random amount
            // 33% chance of subtracting a random amount
            // 33% chance of doing nothing.
            long rand = random(30);
            if (rand >= 10 && rand < 20) {
                AirQuality::tvoc_last = AirQuality::tvoc;
                AirQuality::eco2_last = AirQuality::eco2;
                AirQuality::eco2 = AirQuality::eco2 += random(60);
                AirQuality::tvoc = AirQuality::tvoc += random(14);
                AirQuality::update = true;
                Serial.println("Updating test air quality measurements...");
            } else if (rand >= 20 && rand < 30) {
                AirQuality::tvoc_last = AirQuality::tvoc;
                AirQuality::eco2_last = AirQuality::eco2;
                AirQuality::eco2 = AirQuality::eco2 -= random(84);
                AirQuality::tvoc = AirQuality::tvoc -= random(9);
            }
            airTimer = millis();
            return 1;
        }
        if (!sgp.IAQmeasure())
        {
            Serial.println("Air quality measurement failed.");
            AirQuality::is_error = true;
            return 0;
        }
        AirQuality::eco2 = sgp.eCO2;
        AirQuality::tvoc = sgp.TVOC;

        if (sgp.IAQmeasureRaw())
        {
            Serial.println("Air quality raw Measurement failed");
            return 0;
        }
        AirQuality::rawH2 = sgp.rawH2;
        AirQuality::rawEthanol = sgp.rawEthanol;

        AirQuality::tvoc_last = AirQuality::tvoc;
        AirQuality::eco2_last = AirQuality::eco2;

        AirQuality::CheckAlarms();
        AirQuality::dumpToConsole();
    }
    return 1;
}
uint8_t AirQuality::GetAirQuality(float temperature, float humidity)
{
    if (millis() - airTimer > AirQuality::refresh_rate)
    {
        AirQuality::update = false;
        if (Current_Settings.test_data)
        {
            AirQuality::is_error = false;

            if (AirQuality::tvoc == 0 || isnan(AirQuality::tvoc)) {
                AirQuality::tvoc = Current_Settings.test_TVOC;
            }
            if (AirQuality::eco2 == 0 || isnan(AirQuality::eco2)) {
                AirQuality::eco2 = Current_Settings.test_eCO2;
            }
            if (AirQuality::rawH2 == 0 || isnan(AirQuality::rawH2)) {
                AirQuality::rawH2 = Current_Settings.test_rawH2;
            }
            if (AirQuality::rawEthanol == 0 || isnan(AirQuality::rawEthanol)) {
                AirQuality::rawEthanol = Current_Settings.test_rawEthanol;
            }
            // Generate some random changes in the data.
            // 33% chance of adding a random amount
            // 33% chance of subtracting a random amount
            // 33% chance of doing nothing.
            long rand = random(30);
            if (rand >= 10 && rand < 20) {
                AirQuality::tvoc_last = AirQuality::tvoc;
                AirQuality::eco2_last = AirQuality::eco2;
                AirQuality::eco2 = AirQuality::eco2 += random(60);
                AirQuality::tvoc = AirQuality::tvoc += random(14);
                AirQuality::update = true;
                Serial.println("Updating TEST air quality measurements...");
            } else if (rand >= 20 && rand < 30) {
                AirQuality::tvoc_last = AirQuality::tvoc;
                AirQuality::eco2_last = AirQuality::eco2;
                AirQuality::eco2 = AirQuality::eco2 -= random(84);
                AirQuality::tvoc = AirQuality::tvoc -= random(9);
            }
            airTimer = millis();
            return 1;
        }
        if (!isnan(humidity) && !isnan(temperature))
        {
            if (humidity != humidity_last && temperature != temperature_last)
            {
                sgp.setHumidity(getAbsoluteHumidity(temperature, humidity));
            }
        }
        temperature_last = temperature;
        humidity_last = humidity;

        if (!sgp.IAQmeasure())
        {
            if (!AirQuality::is_error)
            {
                Serial.println("Air quality measurement failed.");
                AirQuality::is_error = true;
                return 1;
            }
            return 1;
        }
        else
        {
            if (AirQuality::is_error)
            {
                Serial.println("Air quality sensor back online.");
                AirQuality::is_error = false;
            }
        }
        AirQuality::eco2 = sgp.eCO2;
        AirQuality::tvoc = sgp.TVOC;

        if (!sgp.IAQmeasureRaw())
        {
            Serial.println("Air quality raw Measurement failed");
            AirQuality::is_error = true;
            return 1;
        }
        AirQuality::rawH2 = sgp.rawH2;
        AirQuality::rawEthanol = sgp.rawEthanol;

        AirQuality::tvoc_last = AirQuality::tvoc;
        AirQuality::eco2_last = AirQuality::eco2;

        if (AirQuality::tvoc > AirQuality::tvoc_high)
        {
            AirQuality::tvoc_high = AirQuality::tvoc;
        }
        if (AirQuality::eco2 > AirQuality::eco2_high)
        {
            AirQuality::eco2_high = AirQuality::eco2;
        }
        if (AirQuality::rawH2 > AirQuality::rawH2_high)
        {
            AirQuality::rawH2_high = AirQuality::rawH2;
        }
        if (AirQuality::rawEthanol > AirQuality::rawEthanol_high)
        {
            AirQuality::rawEthanol_high = AirQuality::rawEthanol;
        }

        airTimer = millis();
        if (AirQuality::alarms_on)
        {
            AirQuality::CheckAlarms();
        }
        AirQuality::dumpToConsole();
    }
    return 0;
}

void AirQuality::CheckAlarms()
{
    if (AirQuality::eco2 >= AirQuality::Current_Settings.warning_eco2_alarm && AirQuality::eco2 < AirQuality::Current_Settings.high_eco2_alarm)
    {
        AirQuality::eco2_warn_active = true;
        AirQuality::eco2_alarm_active = false;
    }
    else if (AirQuality::eco2 >= AirQuality::Current_Settings.high_eco2_alarm)
    {
        AirQuality::eco2_warn_active = false;
        AirQuality::eco2_alarm_active = true;
    }
    else
    {
        AirQuality::eco2_warn_active = false;
        AirQuality::eco2_alarm_active = false;
    }
    if (AirQuality::tvoc >= AirQuality::Current_Settings.warning_tvoc_alarm && AirQuality::tvoc < AirQuality::Current_Settings.high_tvoc_alarm)
    {
        AirQuality::tvoc_warn_active = true;
        AirQuality::tvoc_alarm_active = false;
    }
    else if (AirQuality::tvoc >= AirQuality::Current_Settings.high_tvoc_alarm)
    {
        AirQuality::tvoc_warn_active = false;
        AirQuality::tvoc_alarm_active = true;
    }
    else
    {
        AirQuality::tvoc_warn_active = false;
        AirQuality::tvoc_alarm_active = false;
    }
}
void AirQuality::dumpToConsole() {
    Serial.println("-----|Air Quality|-----");
    Serial.print("eco2: "); Serial.println(AirQuality::eco2);
    Serial.print("eco2_last: "); Serial.println(AirQuality::eco2_last);
    Serial.print("tvoc: "); Serial.println(AirQuality::tvoc);
    Serial.print("tvoc_last: "); Serial.println(AirQuality::tvoc_last);
    Serial.print("rawH2: "); Serial.println(AirQuality::rawH2);
    Serial.print("rawEthanol: "); Serial.println(AirQuality::rawEthanol);
    Serial.print("eco2: "); Serial.println(AirQuality::eco2);
    Serial.println("-----------------------");
}