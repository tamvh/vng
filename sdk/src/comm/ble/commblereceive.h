#ifndef COMMBLERECEIVE_H
#define COMMBLERECEIVE_H


#include <comm/commmessage.h>
#include <ble/bledefs.h>

namespace iot {
namespace comm {
namespace ble {


class SDKSHARED_EXPORT Receive: public comm::Message
{
public:
    Receive(const iot::ble::Address &address,
            const uint8_t *payload,
            const Size &size);

    virtual ~Receive();

    virtual comm::Message::Size size() const;

    virtual comm::Message::Id id() const;

    inline const iot::ble::Payload& payload() const {
        return _payload;
    }

private:
    virtual void fill(uint8_t *payload) const;

private:
    iot::ble::Address _address;
    iot::ble::Payload _payload;
};

} // namespace ble
} // namespace comm
} // namespace iot

#endif // COMMBLERECEIVE_H
