#ifndef BLEDEFS_H
#define BLEDEFS_H
#include <stdint.h>
#define BLE_PAYLOAD_SIZE 31
#define BLE_ADDRESS_SIZE 6
namespace iot {
namespace ble {

typedef uint8_t Payload[BLE_PAYLOAD_SIZE];
typedef uint8_t Address[BLE_ADDRESS_SIZE];

} // namespace ble
} // namespace iot

#endif // BLEDEFS_H
