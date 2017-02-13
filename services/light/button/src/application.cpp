#include "application.h"
#include <core/corevariant.h>
#include <comm/ble/commbleadvertise.h>
#include <comm/ble/commblecommand.h>
#include <comm/ble/commblereceive.h>
#include <ble/gap/blegappacket.h>
#include <ble/gap/blegapevent.h>
#include <net/netchannel.h>
#include <net/netsubscribe.h>
#include <utils/utilsprint.h>
#include <stdio.h>
namespace iot {
class Appication::Private: public net::Subscriber
{
public:

    class Check: public comm::ble::Command
    {
    public:
        Check(const ble::Address &address,
              const uint8_t *payload,
              Private &app):
            comm::ble::Command(address),
            _app(app) {
            memcpy(&_payload[0], payload, BLE_PAYLOAD_SIZE);
        }

        virtual void execute(const ble::Address &address) {
            _app.check(address, &_payload[0]);
        }

    private:
        Private &_app;
        ble::Payload _payload;
    };

    class Accept: public comm::ble::Advertise
    {
    public:
        Accept(uint8_t level):
            _level(level) {

        }

    private:
        virtual void fillPayload(uint8_t *payload) const {
            ble::gap::Packet packet;

            uint8_t flags[] = {0x00};

            packet.setFiled(ble::gap::Packet::TypeFlags, flags, sizeof(flags));

            uint8_t appearance[] = {0};
            packet.setFiled(ble::gap::Packet::TypeAppearance,
                            appearance,
                            sizeof(appearance));

            uint8_t store[sizeof(uint16_t) +
                            sizeof(struct iot::ble::gap::_Event) +
                            sizeof(uint8_t)];

            uint16_t *companyId = (uint16_t *)(&store[0]);
            *companyId = 0xFF0F;

            ble::gap::Event command;
            command = (ble::gap::Event) (&store[0] + sizeof(uint16_t));
            command->commandId = EVENT_LIGHT_LAMP;
            command->gateway = 1;
            uint8_t *level = (uint8_t *)command->payload;
            *level = _level;

            packet.setFiled(ble::gap::Packet::TypeManufacturerSpecificData,
                            store,
                            sizeof(store));

            uint8_t name[] = "VNG lamp";

            packet.setFiled(ble::gap::Packet::TypeCompleteLocalName,
                            name,
                            sizeof(name) - 1);

            memcpy(payload, packet.payload(), packet.length());
        }
    private:
        uint8_t _level;
    };

    Private(net::Channel &channel):
        net::Subscriber(channel) {

    }



    int initialize(const core::Variant &settings) {

        const core::Variant *light = settings.value("light");
        if (light == NULL) {
            PRINTF("*application* initialize: light value missing\r\n");
            return -1;
        }
        const core::Variant *scan = light->value("scan");
        if (scan == NULL) {
            PRINTF("*application* initialize: scan value missing\r\n");
            return -1;
        }

        const core::Variant *scanTopic = scan->value("topic");
        if (scanTopic == NULL) {
            PRINTF("*application* initialize:"
                   " topic value for scan missing\r\n");
            return -1;
        }

        if (scanTopic->type() != core::Variant::TypeString) {
            PRINTF("*application* initialize:"
                   " topic value for scan is not a string\r\n");
            return -1;
        }

        const core::Variant *advertise = light->value("advertise");
        if (advertise == NULL) {
            PRINTF("*application* initialize: advertise value missing\r\n");
            return -1;
        }

        const core::Variant *advertiseTopic = advertise->value("topic");
        if (advertiseTopic == NULL) {
            PRINTF("*application* initialize:"
                   " topic value for advertise missing\r\n");
            return -1;
        }

        if (advertiseTopic->type() != core::Variant::TypeString) {
            PRINTF("*application* initialize:"
                   " topic value for advertise is not a string\r\n");
            return -1;
        }

        _receiveTopic = scanTopic->toString();
        int error = subscribe(_receiveTopic);
        if (error != 0)
            return error;

        const core::Variant *devices = light->value("devices");
        if (devices != NULL)
            initDevides(*devices);

        _advertiseTopic = advertiseTopic->toString();

        return 0;
    }

    void check(const ble::Address &address, const uint8_t *payload) {
        PRINTF("*application* check address: %02X:%02X:%02X:%02X:%02X:%02X\r\n",
               address[0],
               address[1],
               address[2],
               address[3],
               address[4],
               address[5]);

        if (!allowed(address)) {
            PRINTF("*application* rejected\r\n");
            return;
        }

        ble::gap::Packet packet;
        int error  = packet.setPayload(payload, BLE_PAYLOAD_SIZE);
        if (error != 0) {
            PRINTF("*application* set payload failed\r\n");
            return;
        }
        ble::gap::Packet::Field field;
        field = packet.field(ble::gap::Packet::TypeManufacturerSpecificData);

        if (field == NULL) {
            PRINTF("*application* Manufacturer specific data missing\r\n");
            return;
        }
        ble::Payload mfd;
        uint8_t length = field->length - 1;
        memcpy(&mfd[0], field->payload, length);
        uint16_t *companyId = (uint16_t *) &mfd[0];

        if (*companyId != 0xFF0F) { // Treat as byte order need to fix
            PRINTF("*application* company id not macthed\r\n");
            return;
        }

        struct ble::gap::_Event *command;
        command = (struct ble::gap::_Event *) (field->payload + sizeof(uint16_t));
        if (command->commandId != EVENT_LIGHT_BUTTON) {
            PRINTF("*application* command id: 0x%04X not macthed\r\n",
                   command->commandId);
            return;
        }

        uint8_t *level = (uint8_t *) command->payload;
        Accept accept(*level);
        net::Channel::Token token;
        channel().publish(_advertiseTopic, accept, token);
    }

    static bool equal(const ble::Address &address,
                      const std::string &device) {
        if (device.length() != BLE_ADDRESS_SIZE)
            return false;

        return memcmp(&address[0], device.data(), BLE_ADDRESS_SIZE) == 0;
    }

    void release() {
        unsubcrible(_receiveTopic);
        Devices::iterator end = _devices.end();
        for(Devices::iterator iterator = _devices.begin();
            iterator != end;
            iterator ++)
            delete []*iterator;
         _devices.clear();
    }

private:
    static bool toHex(char character, uint8_t &hex) {
        if ('0' <= character && character <= '9') {
            hex = character - '0';
            return true;
        }
        if ('A' <= character && character <= 'F') {
            hex = character - 'A' + 10;
            return true;
        }
        if ('a' <= character && character <= 'a') {
            hex = character - 'a' + 10;
            return true;
        }
        PRINTF("toHex failed\r\n");
        return false;
    }

    static bool toAddress(const std::string &string, uint8_t *address) {

        std::string::size_type length = string.length();
        if (length != 17)
            return false;

        uint8_t position = 0;
        for(std::string::size_type index = 0; index < length; position += 1) {
            uint8_t first;
            if (!toHex(string[index], first))
                return false;

            uint8_t second;
            if (!toHex(string[index + 1], second))
                return false;
            address[position] = (first << 4) + second;
            if (position < 5) {
                if (string[index + 2] != ':')
                    return false;
            }
            index += 3;
        }

        return position == 6;
    }

    static bool equalAddress(const ble::Address &first,
                             const uint8_t *second) {
        for(uint8_t index = 0; index < 6; index++) {
            if (first[index] != second[index])
                return false;
        }
        return true;
    }

    void initDevides(const core::Variant &settings) {
        if (settings.type() != core::Variant::TypeList) {
            PRINTF("setting type %d\r\n", settings.type());
            return;
        }

        core::Variant::List devices = settings.toList();
        core::Variant::List::iterator end = devices.end();
        for(core::Variant::List::iterator iterator = devices.begin();
            iterator != end;
            iterator++) {
            const core::Variant &value = *iterator;
            uint8_t *address = new uint8_t[6];
            std::string string = value.toString();
            if (!toAddress(string, address)) {
                delete address;
                continue;
            }
            _devices.push_back(address);
        }
    }

    bool allowed(const ble::Address &address) {
        Devices::iterator end = _devices.end();
        for(Devices::iterator iterator = _devices.begin();
            iterator != end;
            iterator ++) {
            if (equalAddress(address, *iterator))
                return true;
        }
        PRINTF("*application* not matched address\r\n");
        return false;
    }

    virtual core::Command *parse(const comm::Message::Id &id,
                                 const uint8_t *payload,
                                 const comm::Message::Size &size) {
        PRINTF("*application* parse id: 0x%04X\r\n", id);
        if (id != comm::Message::BleReceive ||
            size < BLE_ADDRESS_SIZE + BLE_PAYLOAD_SIZE)
            return NULL;
        ble::Address address;
        memcpy(&address[0], payload, BLE_ADDRESS_SIZE);
        return new Check(address, payload + BLE_ADDRESS_SIZE, *this);
    }


private:
    Subscriber::Topic _receiveTopic;
    Subscriber::Topic _advertiseTopic;
    typedef std::list<uint8_t *> Devices;
    Devices _devices;
};

Appication::Appication(const std::string &config):
    net::Application(config),
    _private(NULL)
{

}

Appication::~Appication()
{

}

int Appication::initialize(core::Synchronizer& synchronizer,
                           const core::Variant &settings,
                           net::Channel &channel)
{
    if (_private != NULL)
        return -1;
    _private = new Private(channel);
    int error = _private->initialize(settings);
    if (error != 0) {
        delete _private;
        _private = NULL;
        return error;
    }
    return 0;
}

void Appication::release(net::Channel &/*channel*/)
{
    if (_private != NULL)
        return;
    _private->release();
    delete _private;
    _private = NULL;
}
} // namespace iot
