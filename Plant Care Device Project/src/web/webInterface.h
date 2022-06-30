#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <resources/DhtResource.h>
#include <resources/SoilMoistureResource.h>
#include <resources/WaterPump.h>
#include <WiFi.h>
#pragma once

namespace webInterface
{
    namespace _variables
    {
        AsyncWebServer server(80);

        SoilMoistureResource* soil;
        DhtResource* dht;
        TempAndHumidity tah;
        WaterPump* pump;

        SemaphoreHandle_t watering_mutex = xSemaphoreCreateMutex();
        TaskHandle_t watering_handle;
        uint8_t days;

        HTTPClient http;
        uint8_t temperature_average = 0, humidity_average = 0, moisture_average = 0;

    } // namespace _variables

    

    namespace _html
    {
        const char style_center[] = R"rawliteral(
            <style>
                .center {
                    margin: auto;
                    width: 50%%;
                    border: 3px solid green;
                    padding: 10px;
                }
            </style>
        )rawliteral";


        const char html_dashboard[] = R"rawliteral(
            <!DOCTYPE HTML>
            <html>
            <head>
                <title>Dashboard</title>
                %CENTER%
            </head>
            <body>

            <div class="center">
                <h1>Current stats</h1>
            </div>
            <div class="center">
                <p>Current temperature is %TEMPERATURE%C</p>
                <p>Current humidity is %HUMIDITY%%%</p>
                <p>Current soil moisture is %MOISTURE%/10</p>
                <form method="get" action="/today">
                    <button type="submit">See today's analytics</button>
                </form>
                <form method="get" action="/month">
                    <button>See this months analytics</button>
                </form>
                <form method="get" action="/water">
                    <button>Water plant now</button>
                </form>
                <form method="get" action="/schedule">
                    <button>Set watering schedule</button>
                </form>
            </div>
            </body>
            </html>
        )rawliteral";

        const char html_today[] = R"rawliteral(
            <!DOCTYPE HTML>
            <html>
            <head>
                <title>Today's analytics</title>
                %CENTER%
            </head>
            <body>
            <div class="center">
                <h1>Today's analytics</h1>
            </div>
            <div class="center">
                <p>Current temperature average %TEMPERATURE%C</p>
                <p>Current humidity average %HUMIDITY%%%</p>
                <p>Current soil moisture average %MOISTURE%/10</p>
                <p>%MESSAGE%</p>

                <form method="get" action="/month">
                    <button>See this months analytics</button>
                </form>

                <form method="get" action="/">
                    <button type="submit">Back</button>
                </form>
            </div>
            </body>
            </html>
        )rawliteral";

        const char html_month[] = R"rawliteral(
            <!DOCTYPE HTML>
            <html>
            <head>
                <title>Month's analytics</title>
                %CENTER%
            </head>
            <body>
            <div class="center">
                <h1>This month's analytics</h3>
            </div>
            <div class="center">
                <p>Current average temperature %TEMPERATURE%C </p>
                <p>Current average humidity %HUMIDITY%%% </p>
                <p>Current average soil moisture %MOISTURE%/10 </p>
                <p>%MESSAGE%</p>

                <form method="get" action="/today">
                <button>See this today's analytics</button>
                </form>

                <form method="get" action="/">
                    <button type="submit">Back</button>
                </form>
            </div>
            </body>
            </html>
        )rawliteral";

        const char html_watering_schedule[] = R"rawliteral(
            <!DOCTYPE HTML>
            <html>
            <head>
                <title>Schedule watering</title>
                %CENTER%
            </head>
            <body>
            <div class="center">
                <h1>Schedule watering</h1>
            </div>

            <div class="center">
                <label>
                    <form method="get" action="/done" >
                        Water after how many days?
                        <input type="number" value="1" style="width: 35px; margin-left: 5px" min="1" name="days" id="days">
                        <br><br>
                        <button type="submit">Done</button>
                    </form>
                    <form method="get" action="/">
                        <button type="submit">Back</button>
                    </form>

                </label>
            </div>
            </body>
            </html>
        )rawliteral";
    } // namespace _html
    
    namespace _placeholders
    {
        String placeholder_dashboard(const String& var)
        {
            if(var == "TEMPERATURE")
                return String(
                    _variables::tah.temperature
                );
            if(var == "HUMIDITY")
                return String(
                    (uint8_t)_variables::tah.humidity
                );
            if(var == "MOISTURE")
                return String(
                    _variables::soil->xGetMoisture()
                );
            if(var == "CENTER")
                return String(_html::style_center);
            return String();
        }

        String placeholder_today_month(const String& var)
        {
            if(var == "TEMPERATURE")
                return String(_variables::temperature_average);
            if(var == "HUMIDITY")
                return String(_variables::humidity_average);
            if(var == "MOISTURE")
                return String(_variables::moisture_average);
            if(var == "MESSAGE")
                return "Have a wonderful day.";
            if(var == "CENTER")
                return String(_html::style_center);
            return String();
        }
    } // namespace _placeholders

    //WARNING: Only use if WiFi is active.
    void vStartInterface(SoilMoistureResource &soil, DhtResource &dht, WaterPump &pump)
    {
        if(WiFi.status() != WL_CONNECTED)
            return;

        _variables::soil = &soil;
        _variables::dht = &dht;
        _variables::pump = &pump;

        _variables::server.on("/", HTTP_GET, [](AsyncWebServerRequest* r){
            _variables::tah = _variables::dht->xGetTemperatureAndHumidity();
            r->send_P(200, "text/html", _html::html_dashboard, _placeholders::placeholder_dashboard);
        });

        _variables::server.on("/today", HTTP_GET, [](AsyncWebServerRequest* r){
            
            _variables::http.begin(
                get_today_path
            );

            int status = _variables::http.GET();

            if(status <= 299 || status >= 200)
            {

                DynamicJsonDocument doc(1024);
                deserializeJson(doc, _variables::http.getString());
                JsonObject obj = doc.as<JsonObject>();

                _variables::temperature_average = obj[F("temperature")];
                _variables::humidity_average = obj[F("humidity")];
                _variables::moisture_average = obj[F("moisture")];
                
                r->send_P(200, "text/html", _html::html_today, _placeholders::placeholder_today_month);
            }

            else
            {
                r->send(404);
            }
        });

        _variables::server.on("/month", HTTP_GET, [](AsyncWebServerRequest* r){
            
            _variables::http.begin(
                get_month_path
            );

            int status = _variables::http.GET();

            if(status <= 299 || status >= 200)
            {

                DynamicJsonDocument doc(1024);
                deserializeJson(doc, _variables::http.getString());
                JsonObject obj = doc.as<JsonObject>();

                _variables::temperature_average = obj[F("temperature")];
                _variables::humidity_average = obj[F("humidity")];
                _variables::moisture_average = obj[F("moisture")];
                
                r->send_P(200, "text/html", _html::html_month, _placeholders::placeholder_today_month);
            }

            else
            {
                r->send(404);
            }
        });

        _variables::server.on("/schedule", HTTP_GET, [](AsyncWebServerRequest* r){
            
            r->send_P(200, "text/html", _html::html_watering_schedule, _placeholders::placeholder_dashboard);

        });

        _variables::server.on("/done", HTTP_GET, [](AsyncWebServerRequest* r){
            
            if(r->hasParam("days"))
            {
                
                _variables::days = r->getParam("days")->value().toInt();
                xTaskCreatePinnedToCore(
                    [](void* p)
                    {
                        while(1)
                        {
                            if(xSemaphoreTake(_variables::watering_mutex, 0))
                            {
                                vTaskDelay(pdMS_TO_TICKS(1000 * 60 * 60 * 24 * _variables::days));
                                _variables::pump->vWater();
                                xSemaphoreGive(_variables::watering_mutex);
                            }
                        }

                        return;
                    },
                    "Watering schedule task.",
                    1024,
                    NULL,
                    1,
                    &(_variables::watering_handle),
                    1
                );
                r->redirect("/");
            }

            else
            {
                r->send(404);
            }

        });

        _variables::server.on("/water", HTTP_GET, [](AsyncWebServerRequest* r){
            
            xTaskCreatePinnedToCore(
                [](void *p)
                {
                    _variables::pump->vWater();
                    vTaskDelete(NULL);
                    return;
                },
                "/water task.",
                1024,
                NULL,
                1,
                NULL,
                1
            );
            
            r->redirect("/");            
        });

        _variables::server.begin();
    }
    
    
} // namespace webInterface
