#pragma once
#include <PubSubClient.h>

class Mqtt {
public:
    Mqtt(PubSubClient &client)
        : m_client(client)
    {};
    //todo: Copy assignment and destructor
    
    void publish(const int topic_id, const char *message);
private:
    PubSubClient &m_client;
};