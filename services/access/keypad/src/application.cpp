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
using namespace iot::ble::gap;
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
        Accept() {

        }

        virtual ~Accept() {

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
                          sizeof(struct iot::ble::gap::_Event)];

            uint16_t *companyId = (uint16_t *)(&store[0]);
            *companyId = 0xFF0F;

            ble::gap::Event command;
            command = (ble::gap::Event) (&store[0] + sizeof(uint16_t));
            command->commandId = EVENT_ACCESS_DOOR;
            command->gateway = 1;

            packet.setFiled(ble::gap::Packet::TypeManufacturerSpecificData,
                            store,
                            sizeof(store));

            uint8_t name[] = "VNG lamp";

            packet.setFiled(ble::gap::Packet::TypeCompleteLocalName,
                            name,
                            sizeof(name) - 1);

            memcpy(payload, packet.payload(), packet.length());
        }
    };

    Private(net::Channel &channel):
        net::Subscriber(channel) {

    }



    int initialize(const core::Variant &settings) {

        const core::Variant *keypad = settings.value("keypad");
        if (keypad == NULL) {
            PRINTF("*application* initialize: keypad value missing\r\n");
            return -1;
        }
        const core::Variant *scan = keypad->value("scan");
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

        const core::Variant *advertise = keypad->value("advertise");
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

        const core::Variant *password = keypad->value("password");
        if (password == NULL) {
            PRINTF("*application* initialize: password value missing\r\n");
            return -1;
        }

        if (password->type() != core::Variant::TypeString) {
            PRINTF("*application* initialize:"
                   " password value is not a string\r\n");
            return -1;
        }


        _receiveTopic = scanTopic->toString();
        int error = subscribe(_receiveTopic);
        if (error != 0)
            return error;

        _advertiseTopic = advertiseTopic->toString();
        _password = password->toString();
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
        ble::Payload data;
        uint8_t length = field->length - 1;
        if (length < sizeof (uint16_t) +
                     sizeof (uint16_t) +
                     sizeof(struct _Event))
            return;

        memcpy(&data[0], field->payload, length);
        uint16_t *companyId = (uint16_t *) &data[0];
        if (*companyId != 0xFF0F) {
            PRINTF("*application* company id not macthed\r\n");
            return;
        }


        struct _Event *event;

        event = (struct _Event *) (&data[0] + sizeof(uint16_t));
        if (event->commandId != EVENT_ACCESS_KEYPAD) {
            PRINTF("*application* command id: 0x%04X not macthed\r\n",
                   event->commandId);
            return;
        }

        uint8_t passwordLength = length -
                                 sizeof(struct _Event) -
                                 sizeof(uint16_t);
        if (!valided((const char *) event->payload, passwordLength)) {
            PRINTF("*application* password wrong with size: %d, password: ",
                   passwordLength);
            for(uint8_t index = 0; index < passwordLength; index++)
                PRINTF("%c", event->payload[index]);
            PRINTF("\r\n");
            return;
        }

        PRINTF("*application* accepted\r\n");
        Accept accept;
        net::Channel::Token token;
        channel().publish(_advertiseTopic, accept, token);
    }

    bool valided(const char *password, uint8_t length) {
        if (_password.length() != (size_t) length)
            return false;
        for(uint8_t index = 0; index < length; index++)
            if (_password[index] != password[index])
                return false;
        return true;
    }

    void release() {
        unsubcrible(_receiveTopic);
    }

private:
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
    std::string _password;
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
