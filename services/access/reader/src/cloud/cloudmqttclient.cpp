#include "cloudmqttclient.h"
#include "glog/logging.h"
#include <core/corecommand.h>
#include <core/coreapplication.h>

#define QOS 0
class MqttCloudReceived: public iot::core::Command
{
public:
    MqttCloudReceived(const CloudMQTTClient::Received& received, uint8_t* data, int len, char* topic, int topicLen) :
        _receiver(received),
        _len(len)
    {
        char temp[256] = {0};
        memcpy(&temp[0], topic, topicLen);
        temp[topicLen] = 0;
        _topic = std::string(temp);
        _data = new uint8_t[len];
        memcpy(_data, data, len);
    }
    ~MqttCloudReceived()
    {
        delete _data;
    }

    virtual void execute() {
        printf("*MqttCloudReceived* topic =%s\n", _topic.c_str());
        _receiver(_topic, _data, _len);
        delete this;
    }

private:
    std::string     _topic;
    uint8_t*        _data;
    int             _len;
    const CloudMQTTClient::Received& _receiver;
};

CloudMQTTClient::CloudMQTTClient()
{

}

int CloudMQTTClient::initialize(const std::string& client_id, const std::string& ip, int port, const Received& received)
{
    LOG(INFO) << "Cloud mqtt begin init";
    _received = received;

    std::string server_uri = ip + ":" + std::to_string(port);

    int error = MQTTClient_create(&_client,
                                  server_uri.c_str(),
                                  client_id.c_str(),
                                  MQTTCLIENT_PERSISTENCE_NONE,
                                  NULL);
    if (error != MQTTCLIENT_SUCCESS) {
        LOG(INFO) << "Cloud mqtt init fail " << error;
        return error;
    }

    LOG(INFO) << "Cloud mqtt create client success";

    MQTTClient_connectOptions options = MQTTClient_connectOptions_initializer;
    options.keepAliveInterval = 20;
    options.cleansession = 1;
    MQTTClient_setCallbacks(_client,
                            this,
                            connectionLostCB,
                            messageArrivedCB,
                            onDeliveryCompleteCB);
    error = MQTTClient_connect(_client, &options);
    if (error != MQTTCLIENT_SUCCESS) {
        LOG(INFO) << "Cloud mqtt connect failed " << error;
        MQTTClient_destroy(&_client);
        _client = NULL;
        return error;
    }

    LOG(INFO) << "Cloud mqtt connect success";

    return 0;
}

void CloudMQTTClient::uninitialize()
{
    MQTTClient_destroy(&_client);
    _client = NULL;
}

int CloudMQTTClient::subscribe(const std::string &topic)
{
    return MQTTClient_subscribe(_client, topic.c_str(), QOS);
}

void CloudMQTTClient::onConnectionLost(char *cause)
{

}

int CloudMQTTClient::onMessageArrived(char *topic, int topiclen, MQTTClient_message *message)
{
    std::string data((char*)message->payload, message->payloadlen);
    LOG(INFO) << "*application* MQTT cloud receive message: topic = " << topic << ", topic len = " << topiclen << ", data = " << data;
    //_received((uint8_t*)message->payload, message->payloadlen);


    MqttCloudReceived* command = new MqttCloudReceived(_received, (uint8_t*)message->payload, message->payloadlen, topic, topiclen);

    if (iot::core::Application::instance()->schedule(command) != 0) {
        delete command;
    }


    MQTTClient_freeMessage(&message);
    MQTTClient_free(topic);
    return 1;
}

volatile MQTTClient_deliveryToken deliveredtoken;
void CloudMQTTClient::onDeliveryComplete(MQTTClient_deliveryToken token)
{
    deliveredtoken = token;
}

void CloudMQTTClient::connectionLostCB(void *context, char *cause)
{
    CloudMQTTClient *mqttclient = (CloudMQTTClient*) context;
    mqttclient->onConnectionLost(cause);
}

int CloudMQTTClient::messageArrivedCB(void *context, char *topic, int topiclen, MQTTClient_message *message)
{
    CloudMQTTClient *mqttclient = (CloudMQTTClient*) context;
    return mqttclient->onMessageArrived(topic, topiclen, message);
}

void CloudMQTTClient::onDeliveryCompleteCB(void *context, MQTTClient_deliveryToken token)
{
    CloudMQTTClient *mqttclient = (CloudMQTTClient*) context;
    mqttclient->onDeliveryComplete(token);
}
