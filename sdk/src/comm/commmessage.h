#ifndef COMMMESSAGE_H
#define COMMMESSAGE_H

#include <sdkdefs.h>
#include <vector>
#include <stdint.h>

namespace iot {
namespace comm {

class SDKSHARED_EXPORT Message
{
public:
    enum Ids {
        BleReperter= 0x0000,
        BleReceive = 0x0001,
        BleAdvertise = 0x0002
    };

    typedef uint16_t Id;
    typedef uint16_t Size;
    typedef std::vector<uint8_t> Packet;

    Message();

    virtual ~Message();

    void fill(Packet &packet) const;

public:

    virtual Id id() const = 0;

    virtual Size size() const = 0;

private:
    virtual void fill(uint8_t *payload) const = 0;
};

} // namespace comm
} // namespace iot

#endif // COMMMESSAGE_H
