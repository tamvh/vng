#include "cloudprocesscommand.h"
#include "../application.h"
#include <ble/bledefs.h>
#include "controlldeviceadvertise.h"
#include "addressconvertutils.h"
#include <core/corevariant.h>

using namespace iot;

CloudProcessCommand::CloudProcessCommand(std::string topic,
                                         uint8_t* data,
                                         int dataLen,
                                         const DoAdvertiseCB &advertiseCB):
    _doAdvertiseCB(advertiseCB),
    _payload(data, data + dataLen)
{
    _topic = topic;
}

CloudProcessCommand::~CloudProcessCommand()
{
}

void CloudProcessCommand::execute()
{
    std::string data((char*)_payload.data(), _payload.size());
    processCloudMsg(data);

    delete this;
}

int CloudProcessCommand::processCloudMsg(const std::string &data)
{
    bool ok = false;
    core::Variant jobj = core::Variant::loadJsonString(data, &ok);
    if (!ok) {
        return -1;
    }

    if (jobj.type() != core::Variant::TypeMap) {
        return -1;
    }

    const core::Variant* jobjDt = jobj.value("dt");
    if (jobjDt == NULL) {
        return -1;
    }

    const core::Variant* msg_id = jobjDt->value("msg_id");
    if (msg_id == NULL) {
        return -1;
    }

    if (msg_id->type() != core::Variant::TypeInt) {
        return -1;
    }

    const core::Variant* device_id = jobjDt->value("device_id");
    if (device_id == NULL) {
        return -1;
    }

    if (device_id->type() != core::Variant::TypeString) {
        return -1;
    }

    const core::Variant* new_state = jobjDt->value("new_state");
    if (new_state == NULL) {
        return -1;
    }

    if (new_state->type() != core::Variant::TypeInt) {
        return -1;
    }

    int msgId = msg_id->toInt();
    std::string deviceId = device_id->toString();
    int newState = new_state->toInt();

    if (deviceId.length() != BLE_ADDRESS_SIZE*2 + (BLE_ADDRESS_SIZE-1)) {
        PRINTF("*CloudProcessCommand* wrong door id format\n");
        return -1;
    }

    PRINTF("*CloudProcessCommand* topic: %s, check address\r\n", _topic.c_str());
    uint8_t id[BLE_ADDRESS_SIZE] = {0};
    if (AddressConvertUtils::toBLEAddressId(deviceId, id) == false) {
        return -1;
    };

    ble::Address address;
    memcpy(&address[0], &id[0], BLE_ADDRESS_SIZE);

    if (msgId == 1) {
        openDoor(address);
    } else if (msgId == 2) {
        turnLamp(address, newState);
    }

    return 0;
}

void CloudProcessCommand::openDoor(const ble::Address &address) {
    PRINTF("*CloudProcessCommand* openDoor prepare call publish\r\n");
    OpenDoor open_door(address);
    net::Channel::Token token;
    _doAdvertiseCB(open_door, token);
}

void CloudProcessCommand::turnLamp(const ble::Address &address, int brigthness) {
    PRINTF("*CloudProcessCommand* turn lamp prepare call publish\r\n");
    TurnLamp turnLamp(address, brigthness);
    net::Channel::Token token;
    _doAdvertiseCB(turnLamp, token);
}
