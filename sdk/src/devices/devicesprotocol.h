#ifndef DEVICESPROTOCOL_H
#define DEVICESPROTOCOL_H
#include <sdkdefs.h>
#include <core/corecallback.h>
#include <vector>
#include <stdint.h>
namespace iot {
namespace devices {

/**
 * @brief The Protocol class
 */
class SDKSHARED_EXPORT Protocol
{
public:
    typedef uint32_t Size;
    typedef std::vector<uint8_t> Packet;
    typedef core::Callback<void(const uint8_t *packet,
                                const Size &size)> Decoded;


    virtual ~Protocol();


    virtual int encode(const uint8_t *packet,
                       const Size &size,
                       Packet &encoded) = 0;

    virtual int decode(const uint8_t *packet,
                       const Size &size,
                       const Decoded &decoded) = 0;

    virtual void reset() = 0;

};

namespace slbp {

class SDKSHARED_EXPORT Protocol: public devices::Protocol
{
public:

    Protocol();

    virtual ~Protocol();


    virtual int encode(const uint8_t *packet,
                       const Size &size,
                       Packet &encoded);

    virtual int decode(const uint8_t *packet,
                       const Size &size,
                       const Decoded &decoded);

    virtual void reset();

private:
    class Private;
    Private *_private;
};

} // namespace slbp

namespace slip {

class SDKSHARED_EXPORT Protocol: public devices::Protocol
{
public:

    Protocol();

    virtual ~Protocol();


    virtual int encode(const uint8_t *packet,
                       const Size &size,
                       Packet &encoded);

    virtual int decode(const uint8_t *packet,
                       const Size &size,
                       const Decoded &decoded);

    virtual void reset();

private:
    class Private;
    Private *_private;
};

} // namespace slip
} // namespace devices
} // namespace iot
#endif // DEVICESPROTOCOL_H
