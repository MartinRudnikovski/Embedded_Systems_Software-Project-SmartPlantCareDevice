#include <DHTesp.h>
#include <.secrets.h>
#pragma once

class DhtResource
{
private:
    SemaphoreHandle_t mutex;
    TempAndHumidity tah;
    DHTesp dht;
public:
    DhtResource()
    {
        dht.setup(dht_pin, DHTesp::DHT11);
        mutex = xSemaphoreCreateMutex();
    }
    ~DhtResource(){}
    TempAndHumidity xGetTemperatureAndHumidity()
    {
        while (1)
        {
            if(xSemaphoreTake(mutex, 0) == pdTRUE)
            {
                tah = dht.getTempAndHumidity();
                xSemaphoreGive(mutex);
                return tah;
            }
        }
    }
    
};