#ifndef BLEGAPSUBSCRIBE_H
#define BLEGAPSUBSCRIBE_H
#include <net/netsubscribe.h>
namespace iot {
namespace ble {
namespace gap {

class Subscriber: public net::Subscriber
{
public:
    Subscriber();
private:
    virtual core::Command *parse(const comm::Message::Id &id,
                                 const uint8_t *payload,
                                 const Size &size);
};

} // namespace gap
} // namespace ble
} // namespace iot

#endif // BLEGAPSUBSCRIBE_H
