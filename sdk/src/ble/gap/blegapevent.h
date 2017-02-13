#ifndef BLEGAPEVENT_H
#define BLEGAPEVENT_H
#include <stdint.h>
#define EVENT_LIGHT_BUTTON   0x0100
#define EVENT_LIGHT_LAMP     0x0101
#define EVENT_LIGHT_SENSOR   0x2200
#define EVENT_HUM_SENSOR     0x3100
#define EVENT_TEMP_SENSOR    0x3300
#define EVENT_MCTEMP_SENSOR  0x3400

#define EVENT_ACCESS_BUTTON 0x0000
#define EVENT_ACCESS_READER 0x0001
#define EVENT_ACCESS_DOOR   0x0002
#define EVENT_ACCESS_KEYPAD 0x0003

namespace iot {
namespace ble {
namespace gap {

typedef struct _Event
{
    uint16_t sequenceId;
    uint16_t commandId;
    uint8_t gateway;
    uint8_t payload[0];
    uint8_t battery;
} __attribute__((__packed__)) *Event;

} // namespace gap
} // namespace ble
} // namespace iot

#endif // BLEGAPEVENT_H
