#include <comm/ble/commblecommand.h>
#include <string.h>
namespace iot {
namespace comm {
namespace ble {

Command::Command(const iot::ble::Address &address)
{
    memcpy(&_address[0], &address[0], BLE_ADDRESS_SIZE);
}


Command::~Command()
{

}

void Command::execute()
{
    execute(_address);
}

} // namespace ble
} // namespace net
} // namespace iot

