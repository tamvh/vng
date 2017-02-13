#include "application.h"
#include <string.h>
#include <sstream>
#include <functional>
#include <core/corevariant.h>
#include <core/corecommand.h>
#include <net/netaddress.h>
#include <net/netsubscribe.h>
#include <utils/utilsprint.h>
#include <ble/gap/blegappacket.h>

#include <cloudclient.h>
#include <zcloudclient.h>
#include <upload.h>

using namespace iot;
using namespace iot::net::http;
using namespace std;

class Application::Private : public net::Subscriber
{
public:
    Private(net::Channel& channel);

public:
    int initialize(iot::core::Synchronizer &synchronizer,
                   const iot::core::Variant &settings);

    void release(iot::net::Channel &channel);

public:
    //parse incoming mqtt payload returning a Command to execute
    core::Command *parse(const comm::Message::Id &id,
                         const uint8_t *payload,
                         const Size &size);

public:
    std::string _topic;
    CloudClient *_client;
};

Application::Application(const std::string &configFile) :
    iot::net::Application(configFile), _private(NULL)
{
}

int Application::initialize(iot::core::Synchronizer &synchronizer,
                            const iot::core::Variant &settings,
                            iot::net::Channel &channel)
{
    DPRINT("");
    if (_private != NULL)
        return -1;
    _private = new Private(channel);
    int error = _private->initialize(synchronizer, settings);
    if (error != 0) {
        _private->release(channel);
        delete _private;
        _private = NULL;
        return error;
    }
    return 0;
}

void Application::release(net::Channel &channel)
{
    if (_private) {
        _private->release(channel);
        delete _private;
        _private = NULL;
    }
}

Application::Private::Private(net::Channel& channel) :
    iot::net::Subscriber(channel),
    _client(NULL)
{
}

int Application::Private::initialize(iot::core::Synchronizer &synchronizer,
                                     const iot::core::Variant &settings)
{
    const core::Variant *receive = settings.value("receive");
    if (receive == NULL) {
        DPRINT("receive value missing");
        return -1;
    }
    const core::Variant *sub = receive->value("sub");
    if (sub == NULL) {
        DPRINT("sub value missing");
        return -1;
    }
    _topic = sub->toString();
    DPRINT("topic: %s", _topic.c_str());
    const core::Variant *cloud = settings.value("cloud");
    if (cloud == NULL) {
        DPRINT("cloud value missing");
        return -1;
    }
    const core::Variant *cloudType = cloud->value("type");
    if (cloudType == NULL) {
        DPRINT("cloud type missing");
        return -1;
    }
    if (cloudType->toString() == "zcloud") {
        _client = new ZCloudClient();
    } else {
        DPRINT("Not supported cloud type %s", cloudType->toString().c_str());
        return -1;
    }
    int error = _client->initialize(synchronizer, cloud);
    if (error != 0) {
        DPRINT("client initialize failed error %d", error);
        delete _client;
    }

    error = subscribe(_topic);
    if (error != 0) {
        DPRINT("subscribe failed with error %d", error);
        _client->release();
        delete _client;
        _client = NULL;
        return error;
    }

    return error;
}

void Application::Private::release(iot::net::Channel &channel)
{
    unsubcrible(_topic);
    _client->release();
    delete _client;
    _client = NULL;
}

core::Command * Application::Private::parse(const comm::Message::Id & id,
                                            const uint8_t *payload,
                                            const Size &size)
{
    try {
        DPRINT("*application* parse id: 0x%04X\r\n", id);
        if (id != comm::Message::BleReceive ||
            size < BLE_ADDRESS_SIZE + BLE_PAYLOAD_SIZE){
            DPRINT("size invalid");
            return NULL;
        }

        Packet::Field field;
        Packet packet;
        DPRINT("Address: %02X:%02X:%02X:%02X:%02X:%02X"
                , payload[0]
                , payload[1]
                , payload[2]
                , payload[3]
                , payload[4]
                , payload[5]);

        char addr[1024] = {0};
        sprintf(addr, "%x%x%x%x%x%x", payload[0], payload[1], payload[2], payload[3], payload[4], payload[5]);
//        if(strcmp(addr, "ecf1a71301") != 0
//                && strcmp(addr, "f51caa061460") != 0
//                && strcmp(addr, "edec429eb248") != 0
//                && strcmp(addr, "fdedd66c597f") != 0) {
//            DPRINT("address incorrect");
//            return NULL;
//        }

        int error = packet.setPayload(payload + BLE_ADDRESS_SIZE,
                                       - BLE_ADDRESS_SIZE);
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



        if (field->length < sizeof(struct _Event) + sizeof(Upload::Value)) {
            DPRINT("field length");
            return NULL;
        }

        struct _Event *event = (struct _Event *) (field->payload + sizeof(uint16_t));
        printf("Event: \n");
        DPRINT("Length: %d", (int32_t) sizeof(struct _Event));
        for(int32_t i = 0; i < (int32_t) sizeof(struct _Event); i++) {
            printf("%X", event[i]);
        }
        printf("\n");
        DPRINT("commandId: %02x", event->commandId);
        if (event->commandId != EVENT_LIGHT_SENSOR &&
                event->commandId != EVENT_TEMP_SENSOR &&
                event->commandId != EVENT_MCTEMP_SENSOR &&
                event->commandId != EVENT_HUM_SENSOR)
        {
            DPRINT("Not supported command id %02x", *(uint16_t*)event->commandId);
            return NULL;
        }


        DPRINT("gateway: %02x", event->gateway);
        if (event->gateway != 0) {
            return NULL;
        }
        DPRINT("Payload: %X", event->payload);
        Upload::Value value = *(Upload::Value *) event->payload;
        DPRINT("Value: %3.2f", value);
        Upload::Battery battery = event->battery;
        DPRINT("Battery: %d",  event->battery);
        return new Upload(payload, value, battery, event->commandId, _client);
    } catch (exception ex) {
        DPRINT("Exception parse: %s", ex.what());
        return NULL;
    }
    return NULL;
}
