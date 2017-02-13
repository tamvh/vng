#ifndef UPLOAD_H
#define UPLOAD_H

#include <core/corecommand.h>
#include <ble/gap/blegappacket.h>
#include <ble/gap/blegapevent.h>
#include <utils/utilsprint.h>
#include <cloudclient.h>
using namespace iot::ble::gap;
using namespace iot::ble;
class Upload : public iot::core::Command
{
public:
    typedef float Value;
    typedef int Battery;
    typedef uint16_t Type;
    Upload(const uint8_t *address,
           const Value &value,
           const Battery &battery,
           const Type &type,
           CloudClient *client) :
        _client(client),
        _value(value),
        _battery(battery),
        _type(type){
        _address[0] = address[0];
        _address[1] = address[1];
        _address[2] = address[2];
        _address[3] = address[3];
        _address[4] = address[4];
        _address[5] = address[5];
    }

    virtual void execute() {
        try {
            char address[32] = {0};
            sprintf(address,
                    "%02X:%02X:%02X:%02X:%02X:%02X",
                    _address[0],
                    _address[1],
                    _address[2],
                    _address[3],
                    _address[4],
                    _address[5]);
            DPRINT("address: %s\r\n", address);
            _client->deviceUpdate(address, _type, _value, _battery);
        } catch (std::exception ex) {
            DPRINT("exception when execute: %s", ex.what());
        }
        delete this;
    }
public:
    CloudClient *_client;
    Address _address;
    Value _value;
    Battery _battery;
    Type _type;
};

#endif // UPLOAD_H
