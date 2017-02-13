#include <string>
#include "iotcloud.h"
#include <sstream>
#include "cloudprocesscommand.h"
#include <core/coreapplication.h>
#include <net/netaddress.h>
#include <net/paho/netpahochannel.h>
#include <net/mosquitto/netmosquittochannel.h>
#include <utils/utilsprint.h>
#include <fstream>
#include "cloudprocesscommand.h"

using namespace iot;

int IotCloud::initialize( iot::core::Synchronizer& synchronizer, const iot::core::Variant& cloudConfig,
                          const CloudProcessCommand::DoAdvertiseCB& advertiseCB)
{
    _doAdvertiseCB = advertiseCB;
    if (getGatewayId() != 0) {
        DPRINT("*iotcloud* get gateway id fail\r\n");
        return -1;
    }

    const iot::core::Variant *channel = cloudConfig.value("channel");
    if (channel == NULL) {
        DPRINT("*iotcloud* 'cloud.channel' value not found\r\n");
        return -1;
    }

    const core::Variant *addr = channel->value("address");
    if (addr == NULL) {
        DPRINT("*iotcloud* 'cloud.channel.address' value not found\r\n");
        return -1;
    }

    iot::net::Address address;
    int error = address.setup(*addr);
    if (error != 0) {
        DPRINT("*iotcloud* address setup failed with error: %d\r\n", error);
        return error;
    }

    const core::Variant *type = channel->value("type");
    if (type == NULL) {
        DPRINT("*iotcloud* 'cloud.channel.settings.type' value not found\r\n");
        return -1;
    }

    if (type->type() != core::Variant::TypeString) {
        DPRINT("*iotcloud* 'cloud.channel.type' isn't a string\r\n");
        return -1;
    }

    core::Variant::Map map;
    map["id"] = core::Variant(_gw_id);
    core::Variant settings(map);

    std::string string = type->toString();
    if (string == "PAHO") {
        _channel = new iot::net::paho::Channel(address, settings);
    } else if (string == "MOSQUITTO") {
        _channel = new iot::net::mosquitto::Channel(address, settings);
    } else {
        DPRINT("*iotcloud* 'cloud.channel.settings.type' not supported\r\n");
        return -1;
    }

    error = _channel->intialize(synchronizer);
    if (error != 0) {
        DPRINT("*iotcloud* channel initialize failed with error: %d\r\n", error);
        delete _channel;
        _channel = NULL;
        return error;
    }

    std::stringstream topic;
    topic << "vng-cloud/devices/" << _gw_id << "/change_state/request";

    iot::net::Channel::Token token;
    iot::net::Channel::Subscriber subscriber(this, &IotCloud::mqttCloudReceived);
    error = _channel->subscribe(topic.str(), subscriber, token);

    if (error != 0) {
        return error;
    }
    _topics[topic.str()] = token;


    std::stringstream lampTopic;
    lampTopic << "vng-cloud/devices/" << _gw_id << "/switch_on_off/request";

    iot::net::Channel::Token lampToken;
    iot::net::Channel::Subscriber lampSubscriber(this, &IotCloud::mqttCloudReceived);
    error = _channel->subscribe(lampTopic.str(), lampSubscriber, lampToken);
    if (error != 0) {
        return error;
    }
    _topics[lampTopic.str()] = lampToken;

    return 0;
}

void IotCloud::uninitialize()
{
    if (_channel != NULL) {
        for (Topics::iterator it = _topics.begin(); it != _topics.end(); ++it) {
            _channel->unsubscribe(it->first, it->second);
        }
        delete _channel;
    }
}

void IotCloud::setGatewayId(const std::string &gatewayId)
{
    _gw_id = gatewayId;
}

void IotCloud::mqttCloudReceived(const net::Channel::Topic &topic, const net::Channel::Packet &packet)
{
    CloudProcessCommand* command = new CloudProcessCommand(topic, (uint8_t*)packet.data(), packet.size(), _doAdvertiseCB);

    if (command == NULL)
        return;

    if (iot::core::Application::instance()->schedule(command) == 0)
        return;
    delete command;
}

int IotCloud::getGatewayId()
{
    std::ifstream file( "/sys/devices/virtual/net/br-lan/address");
    if ( file ) {
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();

        _gw_id = buffer.str();
        DPRINT("*IotCloud* gateway id = %s\r\n", _gw_id.c_str());
        return 0;
    }
    return -1;
}

IotCloud::IotCloud() :
    _gw_id("")
{
}

IotCloud::~IotCloud()
{
}
