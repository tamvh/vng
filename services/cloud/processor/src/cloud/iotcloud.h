#ifndef IOTCLOUD_H
#define IOTCLOUD_H

#include <string>
#include <thread>
#include <core/corevariant.h>
#include <net/netchannel.h>
#include "cloudprocesscommand.h"

using namespace iot;

//class CloudProcessCommand;
class IotCloud
{
public:
    IotCloud();
    ~IotCloud();

    int initialize( iot::core::Synchronizer& synchronizer, const iot::core::Variant& cloudConfig, const CloudProcessCommand::DoAdvertiseCB& advertiseCB);
    void uninitialize();
    void setGatewayId(const std::string& gatewayId);

protected:
    void mqttCloudReceived(const iot::net::Channel::Topic &topic,
                           const iot::net::Channel::Packet &packet);
    int getGatewayId();

protected:
    std::string _gw_id;

    typedef std::map<iot::net::Channel::Topic, iot::net::Channel::Token> Topics;
    Topics              _topics;
    iot::net::Channel*  _channel;


    CloudProcessCommand::DoAdvertiseCB  _doAdvertiseCB;
};

#endif // IOTCLOUD_H
