#ifndef COMMBLEADVERTISE_H
#define COMMBLEADVERTISE_H
#include <comm/commmessage.h>
namespace iot {
namespace comm {
namespace ble {

class SDKSHARED_EXPORT Advertise: public comm::Message
{
public:
    Advertise();

    virtual ~Advertise();

    virtual comm::Message::Size size() const;

    virtual comm::Message::Id id() const;
private:
    virtual void fill(uint8_t *payload) const;
private:
    virtual void fillPayload(uint8_t *payload) const = 0;
};

} // namespace ble
} // namespace net
} // namespace iot

#endif // COMMBLEADVERTISE_H
