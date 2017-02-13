#ifndef UTILSCRC16_H
#define UTILSCRC16_H
#include <sdkdefs.h>
#include <stdint.h>

namespace iot {
namespace utils {

class SDKSHARED_EXPORT CRC16
{
public:
    typedef uint16_t CRC;
    typedef uint8_t Payload;
    typedef uint32_t Size;

    static CRC calculate(const Payload *payload, const Size &size);

};

} // namespace utils
} // namespace iot
#endif // UTILSCRC16_H
