#include <.secrets.h>
#include <Arduino.h>
#pragma once


class SoilMoistureResource
{
private:
    SemaphoreHandle_t mutex;
    const static uint16_t maximum = 2760;
    const static uint16_t minimum = 1110;
    uint8_t temp;
public:
    SoilMoistureResource()
    {
        pinMode(soil_moisture_pin, INPUT);
        mutex = xSemaphoreCreateMutex();
    }
    uint8_t xGetMoisture()
    {
        while(true)
        {
            if(xSemaphoreTake(mutex, 0) == pdTRUE)
            {
                temp = map(analogRead(soil_moisture_pin), maximum, minimum, 0, 10);
                xSemaphoreGive(mutex);
                return temp;
            }
        }
        
    }
    ~SoilMoistureResource(){}
};