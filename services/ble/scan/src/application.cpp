#include "application.h"
#include <core/corevariant.h>
#include <comm/ble/commblereceive.h>
#include <net/netchannel.h>
#include <ble/bledefs.h>
#include <devices/devicesdevice.h>
#include <devices/devicesprotocol.h>
#include <utils/utilsprint.h>
#include <string.h>
#include <stdio.h>
namespace iot {
class Appication::Private
{
public:
    Private(iot::net::Channel &channel):
        _channel(channel) {

    }


    void received(const uint8_t *packet,
                  const devices::Device::Size &size) {

        if (size < BLE_ADDRESS_SIZE)
            return;

        ble::Address address;
        memcpy(&address[0], packet, BLE_ADDRESS_SIZE);
        PRINTF("Received from address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
               address[0],
                address[1],
                address[2],
                address[3],
                address[4],
                address[5]);

        comm::ble::Receive receive(address,
                                   packet + BLE_ADDRESS_SIZE,
                                   size - BLE_ADDRESS_SIZE);

        net::Channel::Token token;
        _channel.publish(_topic, receive, token);
    }

    int initialize(iot::core::Synchronizer& synchronizer,
                   const iot::core::Variant &settings) {

        const iot::core::Variant *scan = settings.value("scan");
        if (scan == NULL) {
            PRINTF("*application* scan value not found\r\n");
            return -1;
        }

        const iot::core::Variant *topic = scan->value("topic");
        if (topic == NULL) {
            PRINTF("*application* topic value not found\r\n");
            return -1;
        }

        if (topic->type() != iot::core::Variant::TypeString) {
            PRINTF("*application* topic value type: %d is not a string\r\n",
                   (int)topic->type());
            return -1;
        }

        const iot::core::Variant *ble = scan->value("ble");
        if (ble == NULL) {
            PRINTF("*application* ble value not found\r\n");
            return -1;
        }

        int error = initBle(*ble, synchronizer);
        if (error != 0)
            return error;


        _topic = topic->toString();

        PRINTF("*application* initialize successed\r\n");

        return 0;
    }

    void release() {

        _device->close();
        delete _device;
        _device = NULL;

        delete _protocol;
        _protocol = NULL;
    }

private:
    int initBle(const core::Variant &ble,
                iot::core::Synchronizer& synchronizer) {
        // check protocol
        const iot::core::Variant *protocol = ble.value("protocol");
        if (protocol == NULL) {
            PRINTF("*application* ble value not found\r\n");
            return -1;
        }

        int error = initProtocol(*protocol);
        if (error != 0)
            return error;

        const iot::core::Variant *device = ble.value("device");
        if (device == NULL) {
            PRINTF("*application* device value not found\r\n");
            delete _protocol;
            return error;
        }

        error = initDevice(*device, synchronizer);
        if (error != 0) {
            delete _protocol;
            _protocol = NULL;
            return error;
        }

        return 0;
    }

    int initProtocol(const core::Variant &protocol) {
        const iot::core::Variant *value = protocol.value("type");

        if (value == NULL) {
            PRINTF("*application* protocol.type value not found\r\n");
            return -1;
        }

        if (value->type() != core::Variant::TypeString) {
            PRINTF("*application* protocol.type value is not a string\r\n");
            return -1;
        }
        std::string type = value->toString();
        PRINTF("*application* protocol: type: %s\r\n", type.c_str());
        if (type == "SLBP") {
            _protocol = new devices::slbp::Protocol;
            return 0;
        }
        if (type == "SLIP") {
            _protocol = new devices::slip::Protocol;
            return 0;
        }
        PRINTF("*application* protocol: %s is not supported\r\n",
               type.c_str());
        return -2;

    }

    int initDevice(const core::Variant &device,
                   iot::core::Synchronizer& synchronizer) {

        const iot::core::Variant *value = device.value("type");

        if (value == NULL) {
            PRINTF("*application* device.type value not found\r\n");
            return -1;
        }

        if (value->type() != core::Variant::TypeString) {
            PRINTF("*application* device.type value is not a string\r\n");
            return -1;
        }

        std::string type = value->toString();
        value = device.value("path");

        if (value == NULL) {
            PRINTF("*application* device.path value not found\r\n");
            return -1;
        }

        if (value->type() != core::Variant::TypeString) {
            PRINTF("*application* device.path value is not a string\r\n");
            return -1;
        }

        std::string path = value->toString();

        const iot::core::Variant *settings = device.value("settings");
        if (settings == NULL) {
            PRINTF("*application* device.settings value not found\r\n");
            return -1;
        }

        if (type == "SERIAL") {
            _device = new devices::serial::Device;
        } else {
            PRINTF("*application* device type: %s is not supported\r\n",
                   type.c_str());
            return -1;
        }

        int error = _device->open(path.c_str(),
                                  *settings,
                                  *_protocol,
                                  devices::Device::Received(this,
                                                            &Private::received),
                                  synchronizer);
        if (error != 0) {
            delete _device;
            _device = 0;
            return error;
        }
        return 0;
    }


private:
    iot::net::Channel &_channel;
    iot::devices::Device *_device;
    iot::devices::Protocol *_protocol;
    net::Channel::Topic _topic;

};

Appication::Appication(const std::string &config):
    iot::net::Application(config),
    _private(NULL)
{

}

Appication::~Appication()
{

}

int Appication::initialize(iot::core::Synchronizer& synchronizer,
                           const iot::core::Variant &settings,
                           iot::net::Channel &channel)
{
    if (_private != NULL)
        return -1;
    _private = new Private(channel);
    int error = _private->initialize(synchronizer, settings);
    if (error != 0) {
        delete _private;
        _private = NULL;
        return error;
    }
    return 0;
}

void Appication::release(iot::net::Channel &/*channel*/)
{
    if (_private != NULL)
        return;
    _private->release();
    delete _private;
    _private = NULL;
}
} // namespace iot
