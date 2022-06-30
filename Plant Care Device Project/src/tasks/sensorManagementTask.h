#include <HTTPClient.h>
#include <.secrets.h>
#include <LiquidCrystal_I2C.h>
#include <resources/DhtResource.h>
#include <resources/SoilMoistureResource.h>
#include <WiFi.h>

#pragma once

SoilMoistureResource* soilMoistureResource_sensorManagement;
DhtResource* dhtResource_sensorManagement;

void sensorManagementTask(void* p)
{

    float temperature, humidity;
    uint8_t moisture;

    HTTPClient http;

    while (true)
    {
        if(WiFi.status() == WL_CONNECTED)
        {
            TempAndHumidity tah = dhtResource_sensorManagement->xGetTemperatureAndHumidity();
            temperature = tah.temperature;
            humidity = tah.humidity;

            moisture = soilMoistureResource_sensorManagement->xGetMoisture();

            http.begin(String(add_path) + 
            "temperature=" + String(temperature) + 
            "&humidity=" + String(humidity) + 
            "&moisture=" + String(moisture));

            http.GET();
        }
        
        
        vTaskDelay(pdMS_TO_TICKS(1000 * 60 * 30));
    }
    return;
}

//WARNING: Only start task if WiFi is connected.
void vStartSensorManagementTask(SoilMoistureResource &soil, DhtResource &dht)
{
    soilMoistureResource_sensorManagement = &soil;
    dhtResource_sensorManagement = &dht;

    xTaskCreatePinnedToCore(
        sensorManagementTask,
        "Sensor Management Task",
        2048,
        NULL,
        1,
        NULL,
        1
    );
}