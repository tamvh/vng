#ifndef DEVICESDEVICE_H
#define DEVICESDEVICE_H
#include <sdkdefs.h>
#include <core/corecallback.h>
#include <stdint.h>
namespace iot {
namespace core {
class Variant;
class Synchronizer;
}
namespace devices {

class Protocol;

class SDKSHARED_EXPORT Device
{
public:
    typedef uint32_t Id;

    typedef uint32_t Size;

    typedef core::Callback<void(const uint8_t *packet,
                                const Size &size)> Received;

    Device();

    virtual ~Device();

    int open(const char *path,
             const core::Variant &settings,
             Protocol &protocol,
             const Received &received,
             core::Synchronizer &synchronizer);
    int write(const uint8_t *packet, const Size &size);
public:

    virtual void close() = 0;
private:
    virtual int _open(const char *path,
                      const core::Variant &settings,
                      const Received &readed,
                      core::Synchronizer &synchronizer) = 0;
    virtual int _write(const uint8_t *packet, const Size &size) = 0;
private:
    class Private;
    Private *_private;
};

namespace serial {

class SDKSHARED_EXPORT Device: public devices::Device
{
public:
    enum ByteSize {
        ByteSizeFive = 5,
        ByteSizeSix = 6,
        ByteSizeSeven = 7,
        ByteSizeEight = 8
    };

    /*!
     * Enumeration defines the possible parity types for the serial port.
     */
    enum Parity {
        ParityNone = 0,
        ParityOdd = 1,
        ParityEven = 2,
        ParityMark = 3,
        ParitySpace = 4
    };

    /*!
     * Enumeration defines the possible stopbit types for the serial port.
     */
    enum StopBits {
        StopBitsOne = 1,
        StopBitsTwo = 2,
        StopBitsOnePointFive
    };

    /*!
     * Enumeration defines the possible flowcontrol types for the serial port.
     */
    enum FlowControl {
        FlowControlNone = 0,
        FlowControlSoftware,
        FlowControlHardware
    };

    Device();

    virtual ~Device();

public:
    virtual void close();
private:
    virtual int _open(const char *path,
                      const core::Variant &settings,
                      const devices::Device::Received &readed,
                      core::Synchronizer &synchronizer);
    virtual int _write(const uint8_t *packet, const Size &size);
private:
    class Private;
    Private *_private;
};

}
} // namespace devices
} // namespace iot

#endif // DEVICESDEVICE_H
