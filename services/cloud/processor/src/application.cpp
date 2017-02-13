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
#include "cloud/iotcloud.h"
#include <sstream>
#include <iomanip>
#include "cloud/cloudprocesscommand.h"

#define NFC_ID_SIZE 7
using namespace iot::ble::gap;
namespace iot {
class Appication::Private: public net::Subscriber
{
public:
    class Check: public comm::ble::Command
    {
    public:
        Check(const ble::Address &address,
              const uint8_t *id,
              Private &app):
            comm::ble::Command(address),
            _app(app) {
            memcpy(&_id[0], id, NFC_ID_SIZE);
        }

        virtual void execute(const ble::Address &address) {
            _app.check(address, &_id[0]);
            delete this;
        }

    private:
        Private &_app;
        uint8_t _id[NFC_ID_SIZE];
    };

    Private(net::Channel &channel):
        net::Subscriber(channel) {

    }



    int initialize(core::Synchronizer& synchronizer, const core::Variant &settings) {

        const core::Variant *access = settings.value("access");
        if (access == NULL) {
            printf("*application* initialize: access value missing\r\n");
            return -1;
        }
        const core::Variant *scan = access->value("scan");
        if (scan == NULL) {
            printf("*application* initialize: scan value missing\r\n");
            return -1;
        }

        const core::Variant *scanTopic = scan->value("topic");
        if (scanTopic == NULL) {
            printf("*application* initialize:"
                   " topic value for scan missing\r\n");
            return -1;
        }

        if (scanTopic->type() != core::Variant::TypeString) {
            printf("*application* initialize:"
                   " topic value for scan is not a string\r\n");
            return -1;
        }

        const core::Variant *advertise = access->value("advertise");
        if (advertise == NULL) {
            printf("*application* initialize: advertise value missing\r\n");
            return -1;
        }

        const core::Variant *advertiseTopic = advertise->value("topic");
        if (advertiseTopic == NULL) {
            printf("*application* initialize:"
                   " topic value for advertise missing\r\n");
            return -1;
        }

        if (advertiseTopic->type() != core::Variant::TypeString) {
            printf("*application* initialize:"
                   " topic value for advertise is not a string\r\n");
            return -1;
        }

        const core::Variant *cloudInfo = settings.value("cloud");
        if (cloudInfo == NULL) {
            printf("*application* initialize: cloud value missing\r\n");
            return -1;
        }

        /*
        _scanTopic = scanTopic->toString();
        int error = subscribe(_scanTopic);
        if (error != 0)
            return error;
        */

        const core::Variant *ids = access->value("ids");
        if (ids != NULL)
            initIds(*ids);

        _advertiseTopic = advertiseTopic->toString();

        if (_cloud.initialize( synchronizer, *cloudInfo,
                               CloudProcessCommand::DoAdvertiseCB(this, &Appication::Private::doAdvertise)) != 0) {
            printf("*application* initialize: cloud config wrong\r\n");
            return -1;
        }

        return 0;
    }

    void check(const ble::Address &address, const uint8_t *id) {
        PRINTF("*address:"
               " %02X:%02X:%02X:%02X:%02X:%02X:%02X\r\n",
               id[0],
               id[1],
               id[2],
               id[3],
               id[4],
               id[5],
               id[6]);

        std::stringstream ss;
        for (int i = 0; i < NFC_ID_SIZE; i++) {
            ss << std::uppercase << std::hex << std::setfill('0') << std::setw(2) << (unsigned)*(id + i);

            if (i < NFC_ID_SIZE - 1) {
                ss << ":";
            }
        }
        std::string card_id = ss.str();

        char str_address[BLE_ADDRESS_SIZE*2+ + BLE_ADDRESS_SIZE - 1 + 1] = {0};
        sprintf(&str_address[0], "%02X:%02X:%02X:%02X:%02X:%02X",
                                 address[0],
                                 address[1],
                                 address[2],
                                 address[3],
                                 address[4],
                                 address[5]);


    }

    void doAdvertise(const comm::Message &message,
                     net::Channel::Token &token) {
        channel().publish(_advertiseTopic, message, token);
    }

    void release() {
        _cloud.uninitialize();
        //unsubcrible(_scanTopic);
        Ids::iterator end = _ids.end();
        for(Ids::iterator iterator = _ids.begin();
            iterator != end;
            iterator ++)
            delete []*iterator;
         _ids.clear();
    }

public:
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

    static bool toBLEAddressId(const std::string &string, uint8_t *address) {

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
            if (position < BLE_ADDRESS_SIZE - 1) {
                if (string[index + 2] != ':')
                    return false;
            }
            index += 3;
        }

        return position == BLE_ADDRESS_SIZE;
    }

    static bool toId(const std::string &string, uint8_t *address) {

        std::string::size_type length = string.length();
        if (length != 20)
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
            if (position < NFC_ID_SIZE - 1) {
                if (string[index + 2] != ':')
                    return false;
            }
            index += 3;
        }

        return position == NFC_ID_SIZE;
    }

    static bool equalId(const uint8_t *first, const uint8_t *second) {
        for(uint8_t index = 0; index < NFC_ID_SIZE; index++) {
            if (first[index] != second[index])
                return false;
        }
        return true;
    }

private:
    void initIds(const core::Variant &settings) {
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
            uint8_t *id = new uint8_t[NFC_ID_SIZE];
            std::string string = value.toString();
            if (!toId(string, id)) {
                delete id;
                continue;
            }
            _ids.push_back(id);
        }
    }

    bool allowed(const uint8_t *id) {
        Ids::iterator end = _ids.end();
        for(Ids::iterator iterator = _ids.begin();
            iterator != end;
            iterator ++) {
            if (equalId(id, *iterator))
                return true;
        }
        PRINTF("*application* not matched address\r\n");
        return false;
    }

private:
    virtual core::Command *parse(const comm::Message::Id &id,
                                 const uint8_t *payload,
                                 const comm::Message::Size &size) {
        /*
         * Not process message from scan
        */
        return NULL;

        DPRINT("*application* parse id: 0x%04X\r\n", id);
        if (id != comm::Message::BleReceive ||
            size < BLE_ADDRESS_SIZE + BLE_PAYLOAD_SIZE)
            return NULL;

        Packet::Field field;
        Packet packet;
        int error = packet.setPayload(payload + BLE_ADDRESS_SIZE,
                                      size - BLE_ADDRESS_SIZE);
        if (error != 0) {
            DPRINT("*application* parse set payload failed with error: %d", error);
            return NULL;
        }

        field = packet.field(Packet::TypeManufacturerSpecificData);
        if (field == NULL) {
            DPRINT("ManufacturerSpecificData not found");
            return NULL;
        }

        //company id 0xFF0F
        uint16_t companyId = *(uint16_t*) field->payload;
        if (companyId != 0xFF0F) {
            DPRINT("Not supported company id %04x", *(uint16_t*) field->payload);
            return NULL;
        }

        if (field->length < sizeof(struct _Event) + NFC_ID_SIZE) {
            DPRINT("Size: %d is too small",field->length);
            return NULL;
        }

        Event event = (Event) (field->payload + sizeof(uint16_t));

        if (event->type != EVENT_ACCESS_READER) {
            DPRINT("Type: 0x%04X is not compatibled", event->type);
            return NULL;
        }

        if (event->gateway != 0) {
            DPRINT("Not from reader");
            return NULL;
        }


        ble::Address address;
        memcpy(&address[0], payload, BLE_ADDRESS_SIZE);

        return new Check(address, event->payload, *this);
    }

private:
    Subscriber::Topic _scanTopic;
    Subscriber::Topic _advertiseTopic;
    typedef std::list<uint8_t *> Ids;
    Ids _ids;
    IotCloud _cloud;
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
    int error = _private->initialize(synchronizer, settings);
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
