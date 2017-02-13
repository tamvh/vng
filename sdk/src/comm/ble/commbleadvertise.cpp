#include <comm/ble/commbleadvertise.h>
#include <ble/bledefs.h>
#include <string.h>

namespace iot {
namespace comm {
namespace ble {

Advertise::Advertise()
{
}

Advertise::~Advertise()
{

}


comm::Message::Id Advertise::id() const
{
    return comm::Message::BleAdvertise;
}

comm::Message::Size Advertise::size() const
{
    return BLE_PAYLOAD_SIZE;
}

void Advertise::fill(uint8_t *payload) const
{
    memset(payload, 0, BLE_PAYLOAD_SIZE);
    fillPayload(payload);
}
} // namespace ble
} // namespace comm
} // namespace iot
