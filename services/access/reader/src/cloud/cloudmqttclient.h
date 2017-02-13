#ifndef CLOUDMQTTCLIENT_H
#define CLOUDMQTTCLIENT_H

#include <string>
#include "MQTTClient.h"
#include <functional>
#include "core/corecallback.h"

class CloudMQTTClient
{
public:
    typedef iot::core::Callback<void(std::string topic, uint8_t* data, int len)> Received;

    CloudMQTTClient();
    int initialize(const std::string& client_id, const std::string& ip, int port, const Received& received);
    void uninitialize();
    int subscribe(const std::string& topic);

    void onConnectionLost(char *cause);
    int onMessageArrived(char *topic,
                         int length,
                         MQTTClient_message* message);
    void onDeliveryComplete(MQTTClient_deliveryToken token);

private:
    static void connectionLostCB(void *context, char *cause);
    static int messageArrivedCB(void* context,
                              char* topic,
                              int topiclen,
                              MQTTClient_message* message);
    static void onDeliveryCompleteCB(void* context,
                                   MQTTClient_deliveryToken token);

private:
    MQTTClient _client;
    Received _received;

};

#endif // CLOUDMQTTCLIENT_H
