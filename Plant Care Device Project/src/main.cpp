#include <WiFi.h>
#include <tasks/sensorManagementTask.h>
#include <tasks/realTimeSensors.h>
#include <resources/WaterPump.h>
#include <web/webInterface.h>

SoilMoistureResource* soilMoistureResource_global = new SoilMoistureResource();
DhtResource* dhtResource_global = new DhtResource();
WaterPump* water_pump_global = new WaterPump();

void setup() {

  Serial.begin(115200);

  WiFi.begin(ssd, ssd_password);

  while (WiFi.status() != WL_CONNECTED)
  {
    
    delay(500);
  }
  
  

  vStartSensorManagementTask( *soilMoistureResource_global, *dhtResource_global);

  vStartRealTimeSensorsTask(*soilMoistureResource_global, *dhtResource_global);
  
  webInterface::vStartInterface(*soilMoistureResource_global, *dhtResource_global, *water_pump_global);
}

void loop() { vTaskDelete(NULL);}