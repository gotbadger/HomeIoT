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
    Serial.println(buf);
}

//basics needed or all configs
WiFiClient espClient;
PubSubClient client(espClient);
long lastSample = 0;

//temperature
#ifdef TEMP_SENSOR
#include <WEMOS_SHT3X.h>
SHT3X sht30(0x45);
#endif

//setup topics
const char *topics[4] = {
    MQTT_ID "/ip",
    MQTT_ID "/temperature",
    MQTT_ID "/humidity",
    MQTT_ID "/button/0"};
char topics_cache[sizeof(topics)][MQTT_MAX_MESSAGE_SIZE];

void setup_wifi()
{
    // wifi
    WiFi.mode(WIFI_STA);
    WiFi.hostname(MQTT_ID);
    WiFi.begin(NET, PASS);
    LOG_PRINTFLN("\n");
    LOG_PRINTFLN("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
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

void publish_if_changed(const int topic_id, const char *message)
{
    //LOG_PRINTFLN("comparing %s:%s", message, topics_cache[topic_id]);
    //compare the strings see if they are equal
    if (strcmp(message, topics_cache[topic_id]) != 0)
    {
        //update cache with new value limited to max lenth of cache
        strncpy(topics_cache[topic_id], message, sizeof(topics_cache[topic_id]));
        //publish as normal
        publish(topic_id, message);
    }
    else
    {
        //LOG_PRINTFLN("value for %s unchanged", topics[topic_id]);
    }
}

void publish_if_changed(const int topic_id, const float value)
{
    //take only 5 chars. This will give numbers like XX.X when including null terminator
    char buffer[5];
    snprintf(buffer, sizeof(buffer), "%f", value);
    publish_if_changed(topic_id, buffer);
}

void publish_if_changed(const int topic_id, const bool value)
{
    publish_if_changed(topic_id, value ? "1" : "0");
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
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}

bool sample()
{
    //rarther than blocking return true every so often to sample data
    long now = millis();
    if (now - lastSample > MAX_SAMPLE_RATE)
    {
        lastSample = now;
        return true;
    }
    return false;
}

void setup()
{
    // serial
    Serial.begin(HW_UART_SPEED);
    while (!Serial)
        ;
//door & window
#ifdef DOOR_SENSOR
    LOG_PRINTFLN("Door sensor configured on pin %i", D8);
    pinMode(D8, INPUT);
    pinMode(D7, OUTPUT);
#endif
    setup_wifi();
    client.setServer(MQTT_SERVER, 1883);
    client.setCallback(callback);
}

void loop()
{

    if (!client.connected())
    {
        reconnect();
    }

    client.loop();

    if (sample())
    {
#ifdef TEMP_SENSOR
        if (sht30.get() == 0)
        {
            //we only are intrested in publishing changes so...

            publish_if_changed(1, sht30.cTemp);
            publish_if_changed(2, sht30.humidity);
        }
        else
        {
            LOG_PRINTFLN("sht30 read failed");
        }
#endif

#ifdef DOOR_SENSOR
        //NB this only works well for switches that are open/closed for a long time like the magnetic door switch
        digitalWrite(D7, HIGH);
        publish_if_changed(3, digitalRead(D8) == HIGH);
#endif
    }
}