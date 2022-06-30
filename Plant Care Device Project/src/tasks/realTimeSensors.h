#include <LiquidCrystal_I2C.h>
#include <resources/DhtResource.h>
#include <resources/SoilMoistureResource.h>
#include <.secrets.h>
#include <WiFi.h>
#pragma once

SoilMoistureResource* soilMoistureResource_realTime;
DhtResource* dhtResource_realTime;

void _vTaskRealTimeSensor(void* p){
    float temperature, humidity, _temp = -1, _humid = -1;
    uint8_t moisture, _moist = -1, counter = 0;

    LiquidCrystal_I2C lcd(0x27, 16, 2);
    lcd.init();
    lcd.backlight();

    while (true)
    {
        TempAndHumidity tah = dhtResource_realTime->xGetTemperatureAndHumidity();
        temperature = tah.temperature;
        humidity = tah.humidity;

        
        moisture = soilMoistureResource_realTime->xGetMoisture();

        if(_temp != temperature || _humid != humidity || _moist != moisture)
        {
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Tem:" + String(temperature));
            lcd.print(" Hum:" + String((uint8_t)humidity) + "%");
            lcd.setCursor(0, 1);
            lcd.print("Soil mois:" + String(moisture) + "/10");

            _temp = temperature;
            _humid = humidity;
            _moist = moisture;
        }
        vTaskDelay(pdMS_TO_TICKS(1000 * 4));

        if(counter >= 6)
        {
            counter = 0;

            if(WiFi.status() == WL_CONNECTED)
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("User interface:");
                lcd.setCursor(0, 1);
                lcd.print(WiFi.localIP().toString());
            }
            else
            {
                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Device is not");
                lcd.setCursor(0, 1);
                lcd.print("connected.");
            }

            _temp = -1;
            _humid = -1;
            _moist = -1;

            vTaskDelay(pdMS_TO_TICKS(1000 * 10));
        }
        else
        {
            counter++;
        }
    }
    return;
}

void vStartRealTimeSensorsTask(SoilMoistureResource &soil, DhtResource &dht)
{
    soilMoistureResource_realTime = &soil;
    dhtResource_realTime = &dht;

    xTaskCreatePinnedToCore(
        _vTaskRealTimeSensor,
        "Real time sensor management task",
        2048,
        NULL,
        1,
        NULL,
        1
    );
}