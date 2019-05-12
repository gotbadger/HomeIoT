/*
 *******************************************************************************
 * Project URL: https://github.com/gotbadger/HomeIot
 *
 *******************************************************************************
 * Copyright Phil Hayton 2018.

 * Distributed under the MIT License.
 * (See accompanying file LICENSE or copy at http://opensource.org/licenses/MIT)
 *******************************************************************************
 */

// set details here
#define NET "network_name"
#define PASS "password"

#define MQTT_ID "ESP_SENSOR_XX"
#define MQTT_SERVER "192.168.1.165"

#define MQTT_MAX_MESSAGE_SIZE 50
#define MAX_SAMPLE_RATE 2000
#define HW_UART_SPEED 9600L
#define SLEEP_SECONDS 60
#define DEBUG 1

// Include library
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define LOG_PRINTFLN(fmt, ...) logfln(fmt, ##__VA_ARGS__)
#define LOG_SIZE_MAX 128
void logfln(const char *fmt, ...)
{
    char buf[LOG_SIZE_MAX];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, LOG_SIZE_MAX, fmt, ap);
    va_end(ap);
    #ifdef DEBUG
    Serial.println(buf);
    #endif
}

//basics needed or all configs
WiFiClient espClient;
PubSubClient client(espClient);

//temperature
#include <WEMOS_SHT3X.h>
SHT3X sht30(0x45);


//setup topics
const char *topics[4] = {
    MQTT_ID "/ip",
    MQTT_ID "/temperature",
    MQTT_ID "/humidity"};

void setup_wifi()
{
    // wifi
    WiFi.mode(WIFI_STA);
    WiFi.hostname(MQTT_ID);
    WiFi.begin(NET, PASS);
    LOG_PRINTFLN("\n");
 
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(200);
        LOG_PRINTFLN(".");
    }
    LOG_PRINTFLN("Connected to WiFi");
    LOG_PRINTFLN("IP: %s", WiFi.localIP().toString().c_str());
}

// callback for mqtt
void callback(char *topic, byte *payload, unsigned int length)
{
    LOG_PRINTFLN("callback on %s", topic);
}

void publish(const int topic_id, const char *message)
{
    LOG_PRINTFLN("%s %s", topics[topic_id], message);
    client.publish(topics[topic_id], message);
}

void publish(const int topic_id, const float value)
{
    //take only 5 chars. This will give numbers like XX.X when including null terminator
    char buffer[5];
    snprintf(buffer, sizeof(buffer), "%f", value);
    publish(topic_id, buffer);
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        // Attempt to connect
        if (client.connect(MQTT_ID))
        {
            LOG_PRINTFLN("MQTT connected");
            publish(0, WiFi.localIP().toString().c_str());
        }
        else
        {
            LOG_PRINTFLN("MQTT failed sate: %i", client.state());
            //check to see if wifi got killed also
            if (WiFi.status() != WL_CONNECTED)
            {
                LOG_PRINTFLN("WIFI connection failed restarting...");
                //reset and start again
                ESP.reset();
            }
            delay(100);
        }
    }
}


void setup()
{
    // serial
    #ifdef DEBUG
    Serial.begin(HW_UART_SPEED);
    while (!Serial)
        ;
    #endif
    setup_wifi();
    client.setServer(MQTT_SERVER, 1883);
    client.setCallback(callback);
    if (!client.connected())
    {
        reconnect();
    }
    
    LOG_PRINTFLN("Taking readings");
    while(true){
        if (sht30.get() == 0){
            publish(1, sht30.cTemp);
            publish(2, sht30.humidity);
            client.loop();
            delay(1000);
            LOG_PRINTFLN("Sleeping");
            ESP.deepSleep(SLEEP_SECONDS * 1000000);
        }else{
            LOG_PRINTFLN(".");
            delay(200);
        }
    }  
}

void loop() {
}