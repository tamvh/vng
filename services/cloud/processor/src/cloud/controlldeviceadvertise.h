#ifndef CONTROLLDEVICEADVERTISE_H
#define CONTROLLDEVICEADVERTISE_H

#include <utils/utilsprint.h>
#include <comm/ble/commbleadvertise.h>
#include <ble/gap/blegapevent.h>
#include <ble/gap/blegappacket.h>
#include <ble/bledefs.h>
#include <string>

using namespace iot;

class OpenDoor: public comm::ble::Advertise
{
public:

    OpenDoor(const ble::Address& address) {
        memcpy(&_address[0], &address[0], BLE_ADDRESS_SIZE);
    }

private:
    virtual void fillPayload(uint8_t *payload) const {
        DPRINT("*OpenDoor* fill payload\r\n");
        ble::gap::Packet packet;

        uint8_t flags[] = {0x00};

        packet.setFiled(ble::gap::Packet::TypeFlags, flags, sizeof(flags));

        uint8_t appearance[] = {0};
        packet.setFiled(ble::gap::Packet::TypeAppearance,
                        appearance,
                        sizeof(appearance));

        uint8_t store[sizeof(uint16_t) +
                      sizeof(struct iot::ble::gap::_Event) +
                      BLE_ADDRESS_SIZE];

        uint16_t *companyId = (uint16_t *)(&store[0]);
        *companyId = 0xFF0F;

        ble::gap::Event command;
        command = (ble::gap::Event) (&store[0] + sizeof(uint16_t));
        command->type = EVENT_ACCESS_DOOR;
        command->gateway = 1;

        uint8_t *address = (uint8_t *)command->payload;
        memcpy(&address[0], &_address[0], BLE_ADDRESS_SIZE);

        packet.setFiled(ble::gap::Packet::TypeManufacturerSpecificData,
                        store,
                        sizeof(store));

        uint8_t name[] = "VNG door";

        packet.setFiled(ble::gap::Packet::TypeCompleteLocalName,
                        name,
                        sizeof(name) - 1);

        memcpy(payload, packet.payload(), packet.length());
    }
private:
    ble::Address _address;
};

class TurnLamp: public comm::ble::Advertise
{
public:

    TurnLamp(const ble::Address& address, int brightness) {
        memcpy(&_address[0], &address[0], BLE_ADDRESS_SIZE);
        _brightness = brightness;
    }

private:
    virtual void fillPayload(uint8_t *payload) const {
        printf("*application* turn lamp: fill payload _brightness = %d\r\n", _brightness);
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
        command->type = EVENT_LIGHT_LAMP;
        command->gateway = 1;

        uint8_t *address = (uint8_t *)command->payload;
        *address = (uint8_t)_brightness;

        packet.setFiled(ble::gap::Packet::TypeManufacturerSpecificData,
                        store,
                        sizeof(store));

        uint8_t name[] = "VNG Lamp";

        packet.setFiled(ble::gap::Packet::TypeCompleteLocalName,
                        name,
                        sizeof(name) - 1);

        memcpy(payload, packet.payload(), packet.length());
    }
private:
    int          _brightness;
    ble::Address _address;
};


#endif // CONTROLLDEVICEADVERTISE_H
