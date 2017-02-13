#include "application.h"
#include <ble/bledefs.h>
#include <comm/ble/commblecommand.h>
#include <comm/ble/commbleadvertise.h>
#include <ble/gap/blegapevent.h>
#include <ble/gap/blegappacket.h>
#include <core/corevariant.h>
#include <devices/devicesdevice.h>
#include <devices/devicesprotocol.h>
#include <net/netchannel.h>
#include <net/netsubscribe.h>
#include <utils/utilsprint.h>
#include <string.h>
#include <stdio.h>
namespace iot {

/**
 * @brief The Appication::Private class
 */
class Appication::Private: public net::Subscriber
{
public:
    class Advertise: public core::Command
    {
    public:
        Advertise(const uint8_t *payload,
                  uint8_t size,
                  Appication::Private &priv):
            _private(priv){

            memset(&_payload[0], 0, BLE_PAYLOAD_SIZE);
            memcpy(&_payload[0],
                    payload,
                    size < BLE_PAYLOAD_SIZE? size: BLE_PAYLOAD_SIZE);
        }

        virtual ~Advertise() {

        }

    private:
        virtual void execute() {
            _private.advertise(_payload);
            delete this;
        }
    private:
        Appication::Private &_private;
        ble::Payload _payload;
    };

    Private(iot::net::Channel &channel):
        net::Subscriber(channel),
        _channel(channel) {

    }


    void received(const uint8_t *packet,
                const devices::Device::Size &size) {

    }

    int initialize(iot::core::Synchronizer& synchronizer,
                   const iot::core::Variant &settings) {

        PRINTF("*application* start initialize\r\n");
        const iot::core::Variant *advertise = settings.value("advertise");
        if (advertise == NULL) {
            PRINTF("*application* scan value not found\r\n");
            return -1;
        }

        const iot::core::Variant *topic = advertise->value("topic");
        if (topic == NULL) {
            PRINTF("*application* topic value not found\r\n");
            return -1;
        }

        if (topic->type() != iot::core::Variant::TypeString) {
            PRINTF("*application* topic value type: %d is not a string\r\n",
                   (int)topic->type());
            return -1;
        }

        const iot::core::Variant *ble = advertise->value("ble");
        if (ble == NULL) {
            PRINTF("*application* ble value not found\r\n");
            return -1;
        }

        int error = initBle(*ble, synchronizer);
        if (error != 0)
            return error;


        _topic = topic->toString();
        error = subscribe(_topic);

        if (error != 0) {
            _device->close();
            delete _device;
            _device = NULL;
            delete _protocol;
            _protocol = NULL;
            return error;
        }

        PRINTF("*application* initialize successed\r\n");
        return 0;
    }

    void advertise(const ble::Payload &payload) {
        int error = _device->write(&payload[0], BLE_PAYLOAD_SIZE);
        if (error != 0) {
            PRINTF("*application* advertising...");
            PRINTF("failed with error: %d\r\n", error);
            return;
        }
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

        devices::Device::Received received(this, &Private::received);

        int error = _device->open(path.c_str(),
                                  *settings,
                                  *_protocol,
                                  received,
                                  synchronizer);
        if (error != 0) {
            delete _device;
            _device = 0;
        }
        return 0;
    }

    virtual core::Command *parse(const comm::Message::Id &id,
                                 const uint8_t *payload,
                                 const Size &size) {
        PRINTF("*application* parse payload with command id: 0x%04X\r\n", id);
        if (id != comm::Message::BleAdvertise) {
            PRINTF("*application* not matched\r\n");
            return NULL;
        }
        PRINTF("*application* matched\r\n");
        return new Advertise(payload, size, *this);
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
