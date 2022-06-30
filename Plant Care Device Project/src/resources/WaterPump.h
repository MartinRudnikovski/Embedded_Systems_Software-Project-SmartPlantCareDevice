#pragma once
#include <Arduino.h>
#include <.secrets.h>

class WaterPump
{
private:
    SemaphoreHandle_t mutex;
    TaskHandle_t handle;
public:

    WaterPump()
    {
        pinMode(water_pump_pin, OUTPUT);
        mutex = xSemaphoreCreateMutex();
    }


    void vWater()
    {
        if(xSemaphoreTake(mutex, 0))
        {
            digitalWrite(water_pump_pin, HIGH);
            vTaskDelay(pdMS_TO_TICKS(1000 * 3));
            digitalWrite(water_pump_pin, LOW);
            xSemaphoreGive(mutex);
        }
    }
    ~WaterPump();
};