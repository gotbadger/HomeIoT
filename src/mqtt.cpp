#include "config.h"
#include "mqtt.h"
#include "log.h"

static const char *topics[4] = {
    MQTT_ID "/ip",
    MQTT_ID "/temperature",
    MQTT_ID "/humidity",
    MQTT_ID "/button/0"};
//static char topics_cache[sizeof(topics)][MQTT_MAX_MESSAGE_SIZE];

void Mqtt::publish(const int topic_id, const char *message){
    LOG_PRINTFLN("%s %s", topics[topic_id], message);
    m_client.publish(topics[topic_id], message);
}