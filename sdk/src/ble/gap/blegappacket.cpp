#include <ble/gap/blegappacket.h>
#include <string.h>
#include <stdio.h>
#include <sstream>
#include <functional>

namespace iot {
namespace ble {
namespace gap {

class Packet::Private
{
public:
    Private():
        _length(0) {
        memset(&_payload[0], 0, sizeof(_payload));
    }

    int setPayload(const Payload &payload) {
        return setPayload(&payload[0], BLE_PAYLOAD_SIZE);
    }

    int setPayload(const uint8_t *payload, uint8_t size) {
        try {
            memset(&_payload[0], 0, BLE_PAYLOAD_SIZE);
            printf("*Gap packet* set payload with size: %d\r\n", size);

            for(_length = 0; _length < size - 2 &&
                             _length < BLE_PAYLOAD_SIZE - 2;) {

                Packet::Field source = (Packet::Field)(payload + _length);


                if (source->type == 0) {
                    return 0;
                }

                if (source->length < 1) {
                    return -1;
                }

                if (_length + source->length + 1 > size) {
                    return -1;
                }

                Packet::Field target = (Packet::Field)(&_payload[_length]);
                target->type = source->type;
                target->length = source->length;

                if (source->length - 1 > 0) {
                    memcpy(&target->payload[0],
                           &source->payload[0],
                           source->length - 1);
                }

                _length += source->length + 1;
            }
        } catch (std::exception ex) {
            return -1;
        }


        return 0;
    }


    int setField(uint8_t type, const uint8_t *payload, uint8_t length) {
        Packet::Field field = this->field(type);
        if (field == NULL) {
            if (_length + length + 2> BLE_PAYLOAD_SIZE)
                return -1;
        } else {
            int required = length + 2;
            required += _length;
            required -= field->length + 1;

            if (required > BLE_PAYLOAD_SIZE)
                return -1;

            remove(field);
        }

        field = (Packet::Field)&_payload[_length];
        field->length = length + 1;
        field->type = type;
        memcpy(&field->payload[0], payload, length);
        _length += field->length + 1;
        return 0;
    }

    void remove(Packet::Field field) {
        uint8_t *payload = (uint8_t *)field;
        uint8_t start = (payload - &_payload[0]);
        uint8_t length = _length - start - field->length;
        for(uint8_t index = 0; index < length; index++)
            payload[index] = payload[index + field->length];
        payload[start + length] = 0;
    }


    Packet::Field field(uint8_t type) {
        try {
            for (uint8_t position = 0; position < BLE_PAYLOAD_SIZE - 2;) {
                Packet::Field field = (Packet::Field) &_payload[position];
                if (field->type == 0)
                    return NULL;
                if (field->type == type)
                    return field;

                position += field->length + 1;
            }
        } catch (std::exception ex) {
            return NULL;
        }

        return NULL;
    }

    inline const uint8_t *payload() const {
        return &_payload[0];
    }

    inline uint8_t length(void) const {
        return _length;
    }

private:
    Payload _payload;
    uint8_t _length;
};

Packet::Packet():
    _private(new Private)
{

}


Packet::~Packet()
{
    delete _private;
}

int Packet::setPayload(const Payload &payload)
{
    return _private->setPayload(payload);
}

int Packet::setPayload(const uint8_t *payload, uint8_t size)
{
    return _private->setPayload(payload, size);
}


int Packet::setFiled(uint8_t type, const uint8_t *payload, uint8_t length)
{
    return _private->setField(type, payload, length);
}


Packet::Field Packet::field(uint8_t type)
{
    return _private->field(type);
}

const uint8_t *Packet::payload() const
{
    return _private->payload();
}

uint8_t Packet::length(void) const
{
    return _private->length();
}

} // namespace gap
} // namespace ble
} // namespace iot
