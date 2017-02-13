#ifndef IOTCLOUD_H
#define IOTCLOUD_H

#include <string>
#include <thread>
#include "cloud/cloudmqttclient.h"
#include <core/corevariant.h>

using namespace iot;
//class GwMqttClient;
class CloudMQTTClient;
/*
 * Connection to iot cloud
 * - Send device event
 * - Receive mqtt message
 */
#define COMMANDID_CLOUD 0xFF00
class IotCloud
{
public:
    IotCloud();
    ~IotCloud();
    bool sendStaffCard(const std::string& reader_id, const std::string& card_id);

    int initializeConfig(const iot::core::Variant *cloudConfig);
    void initialize(const CloudMQTTClient::Received& _received);
    void uninitialize();
    void setServerApiUrl(const std::string& serverUrl);
    void setGatewayId(const std::string& gatewayId);
    void setCloudMqtt(const std::string& host, int port);

protected:
    void processCloudMessage(uint8_t* data, int len);

    std::string _server_url;
    std::string _gw_id;
    std::string _cloud_mqtt_host;
    int         _cloud_mqtt_port;
    CloudMQTTClient *mqtt_cloud;
};

#endif // IOTCLOUD_H
