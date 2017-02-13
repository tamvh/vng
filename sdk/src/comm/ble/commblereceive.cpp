#include <comm/ble/commblereceive.h>
#include <string.h>
namespace iot {
namespace comm {
namespace ble {

Receive::Receive(const iot::ble::Address &address,
                 const uint8_t *payload,
                 const Size &size) {
    memcpy(&_address[0], &address[0], BLE_ADDRESS_SIZE);
    memcpy(&_payload[0], payload, size > BLE_PAYLOAD_SIZE? BLE_PAYLOAD_SIZE:
                                                           size);
}

Receive::~Receive()
{

}


comm::Message::Id Receive::id() const
{
    return comm::Message::BleReceive;
}

void Receive::fill(uint8_t *payload) const
{
    memcpy(&payload[0], &_address[0], BLE_ADDRESS_SIZE);
    memcpy(&payload[BLE_ADDRESS_SIZE], &_payload[0], BLE_PAYLOAD_SIZE);
}

comm::Message::Size Receive::size() const
{
    return BLE_PAYLOAD_SIZE + BLE_ADDRESS_SIZE;
}

} // namespace ble
} // namespace comm
} // namespace iot
