
#include <devices/devicesdevice.h>
#include <devices/devicesprotocol.h>
#include <core/corecommand.h>
#include <core/coresynchronize.h>
#include <core/corevariant.h>
#include <utils/utilsprint.h>
#include <vector>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

namespace iot {
namespace devices {
class Device::Private
{
public:
    Private(Protocol &protocol,
            const Device::Received &received):
        _protocol(protocol),
        _received(received) {

    }

    void decoded(const uint8_t *packet, const Protocol::Size &size) {
        _received.call(packet, size);
    }

    void received(const uint8_t *packet, const Device::Size &size) {
//        PRINTF("*device* received size: %d\r\n", size);
        _protocol.decode(packet,
                         size,
                         Protocol::Decoded(this, &Private::decoded));
    }

    inline Protocol &protocol() {
        return _protocol;
    }

private:
    Protocol &_protocol;
    Device::Received _received;
};
Device::Device():
    _private(NULL)
{

}

Device::~Device()
{

}

int Device::open(const char *path,
                 const core::Variant &settings,
                 Protocol &protocol,
                 const Received &received,
                 core::Synchronizer &synchronizer)
{
    if (_private != NULL)
        return -1;
    _private = new Private(protocol, received);
    int error = _open(path,
                      settings,
                      Received(_private, &Private::received),
                      synchronizer);
    if (error != 0) {
        delete _private;
        _private = NULL;
        return error;
    }
    return 0;
}

int Device::write(const uint8_t *packet, const Size &size)
{
    Protocol::Packet encoded;
    int error = _private->protocol().encode(packet, size, encoded);
    if (error != 0)
        return error;

    return _write(encoded.data(), encoded.size());
}

namespace serial {
class Device::Private: public core::Synchronizer::Callback
{
public:

    class Send
    {
    public:
        Send(const uint8_t* packet, const devices::Device::Size &size):
            _packet(new uint8_t[size]),
            _size(size),
            _written(0) {
            memcpy(_packet, packet, size);
        }


        virtual ~Send() {
            delete []_packet;
        }

        void write(int device) {
            _written += ::write(device, _packet + _written, _size - _written);
//            PRINTF("\r\n*serial* remains size: %d\r\n", _size - _written);
            tcdrain(device);
        }

        inline bool completed() const {
            return _written >= _size;
        }

    private:
        uint8_t* _packet;
        devices::Device::Size _size;
        devices::Device::Size _written;

    };

    Private(const Device::Received &readed, core::Synchronizer &synchronizer):
        _readed(readed),
        _synchronizer(synchronizer) {

    }

    virtual void read() {
        uint8_t packet[128];
        devices::Device::Size size;
        do {
            size = ::read(_device, &packet[0], 128);
//            for(devices::Device::Size index = 0; index < size; index ++)
//                PRINTF("%c", packet[index]);
//            PRINTF("\r\n");

//            for(devices::Device::Size index = 0; index < size; index ++)
//                PRINTF("%02X", packet[index]);
//            PRINTF("\r\n");
            _readed.call(&packet[0], size);
        } while(size == 128);
    }


    virtual bool wantRead() const {
        return true;
    }

    virtual void write() {
        do {
            Send *pending = *_pendings.begin();
            pending->write(_device);
            if (!pending->completed())
                break;
            _pendings.pop_front();
            delete pending;
        } while(!_pendings.empty());
        _synchronizer.update(_device);
    }

    virtual bool wantWrite() const {
        return _pendings.size() > 0;
    }

    virtual void timeout() {

    }

    int open(const char *path, const core::Variant &settings) {

        int error = initialize(settings);
        if (error != 0)
            return error;

        _device = ::open(path, O_RDWR | O_NOCTTY | O_NONBLOCK);
        if (_device == -1) {
            PRINTF("*serial* open failed with error: %d", errno);
            return errno;
        }
        error = configure();
        if (error != 0) {
            ::close(_device);
            _device = 0;
            PRINTF("*serial* setup failed with error: %d", errno);
            return error;
        }

        error = _synchronizer.append(_device, this);
        if (error != 0) {
            ::close(_device);
            _device = 0;
            PRINTF("*serial* synchonize failed with error: %d", errno);
            return error;
        }

        return 0;
    }

    int write(const uint8_t *packet, const devices::Device::Size &size) {

        bool empty = _pendings.empty();
        Send *task = new Send(packet, size);
        _pendings.push_back(new Send(packet, size));

        DPRINT("\r\n*serial* write source: ");
        for(int index = 0; index < size; index++)
            DPRINT("%02X", packet[index]);
        if (!empty)
            return 0;
        int error = _synchronizer.update(_device);
        if (error != 0) {
            _pendings.pop_back();
            delete task;
            return error;
        }
        return 0;
    }

    void close() {
        _synchronizer.remove(_device);
        ::close(_device);
        _device = 0;
    }
private:
    int initialize(const core::Variant &settings) {
        // parse baud rate
        const core::Variant *baudRate = settings.value("baudrate");
        if (baudRate == NULL) {
            _baudRate = B115200;
        } else {
            if (!baudRate->isInteger()) {
                PRINTF("*serial* 'baudrate' is't an integer value\r\n");
                return -1;
            }
            _baudRate = Private::baudRate(baudRate->toInt());
            PRINTF("*serial* initialize with baud rate: %d:\r\n",
                   baudRate->toInt());
            if (_baudRate == -1) {
                PRINTF("*serial* baud rate: %d not supported\r\n",
                       baudRate->toInt());
                return -1;
            }
        }

        // parse parity
        const core::Variant *parity = settings.value("parity");
        if (parity == NULL) {
            _parity = ParityNone;
        } else {
            if (parity->type() != core::Variant::TypeString) {
                PRINTF("*serial* 'parity' is't a string value\r\n");
                return -1;
            }
            std::string string = parity->toString();
            PRINTF("*serial* initialize with bit parity: %s:\r\n",
                   string.c_str());
            if (string == "none") {
                _parity = Device::ParityNone;
            } else if (string == "odd") {
                _parity = Device::ParityOdd;
            } else if (string == "even") {
                _parity = Device::ParityEven;
            } else if (string == "mark") {
                _parity = Device::ParityMark;
            } else if (string == "space") {
                _parity = Device::ParitySpace;
            } else {
                PRINTF("*serial* 'parity' not supported\r\n");
                return -1;
            }
        }

        // parse byte size
        const core::Variant *byteSize = settings.value("bytesize");
        if (byteSize == NULL) {
            _byteSize = Device::ByteSizeEight;
        } else {
            if (!byteSize->isInteger()) {
                PRINTF("*serial* 'bytesize' is't a integer value\r\n");
                return -1;
            }

            PRINTF("*serial* initialize with byte size: %d:\r\n",
                   byteSize->toInt());

            switch (byteSize->toInt()) {
            case 5:
                _byteSize = Device::ByteSizeFive;
                break;
            case 6:
                _byteSize = Device::ByteSizeSix;
                break;
            case 7:
                _byteSize = Device::ByteSizeSeven;
                break;
            case 8:
                _byteSize = Device::ByteSizeEight;
                break;
            default:
                PRINTF("*serial* 'bytesize' not supported\r\n");
                return -1;
            }
        }

        // parse stop bits
        const core::Variant *stopBits = settings.value("stopbits");
        if (stopBits == NULL) {
            _stopBits = Device::StopBitsOne;
        } else {
            if (stopBits->type() != core::Variant::TypeString) {
                PRINTF("*serial* 'stopbits' is't a string value\r\n");
                return -1;
            }

            std::string string = stopBits->toString();

            PRINTF("*serial* initialize with stop bits: %s:\r\n",
                   string.c_str());

            if (string == "one") {
                _stopBits = Device::StopBitsOne;
            } else if (string == "two") {
                _stopBits = Device::StopBitsTwo;
            } else if (string == "onepointfive") {
                _stopBits = Device::StopBitsOnePointFive;
            } else {
                PRINTF("*serial* 'stopbits' not supported\r\n");
                return -1;
            }
        }

        // parse stop bits
        const core::Variant *flowControl = settings.value("flowcontrol");
        if (flowControl == NULL) {
            _flowControl = Device::FlowControlNone;
        } else {
            if (flowControl->type() != core::Variant::TypeString) {
                PRINTF("*serial* 'flowcontrol' is't a string value\r\n");
                return -1;
            }
            std::string string = flowControl->toString();
            PRINTF("*serial* initialize with flow control: %s\r\n",
                   string.c_str());
            if (string == "none") {
                _flowControl = Device::FlowControlNone;
            } else if (string == "hardware") {
                _flowControl = Device::FlowControlHardware;
            } else if (string == "software") {
                _flowControl = Device::FlowControlSoftware;
            } else {
                PRINTF("*serial* 'flowcontrol' not supported\r\n");
                return -1;
            }
        }

        return 0;
    }

    /**
     * @brief setup
     * @param settings
     * @return
     */
    int configure() {
        struct termios options; // The options for the file descriptor
        if (tcgetattr(_device, &options) == -1) {
            PRINTF("*serial* get attributes failed\r\n");
            return -1;
        }

        // set up raw mode / no echo / binary
        options.c_cflag |= (tcflag_t)  (CLOCAL | CREAD);
        options.c_lflag &= (tcflag_t) ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL |
                                        ISIG | IEXTEN); //|ECHOPRT

        options.c_oflag &= (tcflag_t) ~(OPOST);
        options.c_iflag &= (tcflag_t) ~(INLCR | IGNCR | ICRNL | IGNBRK);
#ifdef IUCLC
        options.c_iflag &= (tcflag_t) ~IUCLC;
#endif
#ifdef PARMRK
        options.c_iflag &= (tcflag_t) ~PARMRK;
#endif

#ifdef _BSD_SOURCE
        ::cfsetspeed(&options, _baudRate);
#else
        ::cfsetispeed(&options, _baudRate);
        ::cfsetospeed(&options, _baudRate);
#endif

        // setup char len
        options.c_cflag &= (tcflag_t) ~CSIZE;

        switch (_byteSize) {
        case Device::ByteSizeEight:
            options.c_cflag |= CS8;
            break;
        case Device::ByteSizeSeven:
            options.c_cflag |= CS7;
            break;
        case Device::ByteSizeSix:
            options.c_cflag |= CS6;
            break;
        case Device::ByteSizeFive:
            options.c_cflag |= CS5;
            break;
        }

        // setup stopbits
        switch (_stopBits) {
        case Device::StopBitsOne:
            options.c_cflag &= (tcflag_t) ~(CSTOPB);
            break;
        case Device::StopBitsOnePointFive:
            options.c_cflag |=  (CSTOPB);
            break;
        case Device::StopBitsTwo:
            options.c_cflag |=  (CSTOPB);
            break;
        }

        // setup parity
        options.c_iflag &= (tcflag_t) ~(INPCK | ISTRIP);
        switch (_parity) {
        case Device::ParityNone:
            options.c_cflag &= (tcflag_t) ~(PARENB | PARODD);
            break;
        case Device::ParityEven:
            options.c_cflag &= (tcflag_t) ~(PARODD);
            options.c_cflag |=  (PARENB);
            break;
        case Device::ParityOdd:
            options.c_cflag |=  (PARENB | PARODD);
            break;
        case Device::ParityMark:
#ifdef CMSPAR
            options.c_cflag |=  (PARENB | CMSPAR | PARODD);
            break;
#else
            PRINTF("*serial* setup parity not supported\r\n");
            return -1;
#endif
        case Device::ParitySpace:
#ifdef CMSPAR
            options.c_cflag |=  (PARENB | CMSPAR);
            options.c_cflag &= (tcflag_t) ~(PARODD);
            break;
#else
            PRINTF("*serial* setup parity not supported\r\n");
            return -1;
#endif
        }

        // setup flow control
        bool xonXoff;
        bool rtsCts;

        if (_flowControl == Device::FlowControlNone) {
            xonXoff = false;
            rtsCts = false;
        } else if (_flowControl == Device::FlowControlSoftware) {
            xonXoff = true;
            rtsCts = false;
        } else  {
            xonXoff = false;
            rtsCts = true;
        }

        // xonxoff
#ifdef IXANY
        if (xonXoff)
            options.c_iflag |=  (IXON | IXOFF); //|IXANY)
        else
            options.c_iflag &= (tcflag_t) ~(IXON | IXOFF | IXANY);
#else
        if (xonXoff)
            options.c_iflag |=  (IXON | IXOFF);
        else
            options.c_iflag &= (tcflag_t) ~(IXON | IXOFF);
#endif
        // rtscts
#ifdef CRTSCTS
        if (rtsCts)
            options.c_cflag |=  (CRTSCTS);
        else
            options.c_cflag &= (unsigned long) ~(CRTSCTS);
#elif defined CNEW_RTSCTS
        if (rtsCts)
            options.c_cflag |=  (CNEW_RTSCTS);
        else
            options.c_cflag &= (unsigned long) ~(CNEW_RTSCTS);
#else
#endif

        // http://www.unixwiz.net/techtips/termios-vmin-vtime.html
        // this basically sets the read call up to be a polling read,
        // but we are using select to ensure there is data available
        // to read before each call, so we should never needlessly poll
        options.c_cc[VMIN] = 0;
        options.c_cc[VTIME] = 0;

        // activate settings
        if (::tcsetattr (_device, TCSANOW, &options) == 0)
            return 0;
        return errno;
    }

    /**
     * @brief baudRate
     * @param rate
     * @return
     */
    static speed_t baudRate(unsigned int rate) {
        // setup baud rate
        switch (rate) {
#ifdef B0
        case 0: return B0;
#endif
#ifdef B50
        case 50: return B50;
#endif
#ifdef B75
        case 75: return B75;
#endif
#ifdef B110
        case 110: return B110;
#endif
#ifdef B134
        case 134: return B134;
#endif
#ifdef B150
        case 150: return B150;
#endif
#ifdef B200
        case 200: return B200;
#endif
#ifdef B300
        case 300: return B300;
#endif
#ifdef B600
        case 600: return B600;
#endif
#ifdef B1200
        case 1200: return B1200;
#endif
#ifdef B1800
        case 1800: return B1800;
#endif
#ifdef B2400
        case 2400: return B2400;
#endif
#ifdef B4800
        case 4800: return B4800;
#endif
#ifdef B7200
        case 7200: return B7200;
#endif
#ifdef B9600
        case 9600: return B9600;
#endif
#ifdef B14400
        case 14400: return B14400;
#endif
#ifdef B19200
        case 19200: return B19200;
#endif
#ifdef B28800
        case 28800: return B28800;
#endif
#ifdef B57600
        case 57600: return B57600;
#endif
#ifdef B76800
        case 76800: return B76800;
#endif
#ifdef B38400
        case 38400: return B38400;
#endif
#ifdef B115200
        case 115200: return B115200;
#endif
#ifdef B128000
        case 128000: return B128000;
#endif
#ifdef B153600
        case 153600: return B153600;
#endif
#ifdef B230400
        case 230400: return B230400;
#endif
#ifdef B256000
        case 256000: return B256000;
#endif
#ifdef B460800
        case 460800: return B460800;
#endif
#ifdef B576000
        case 576000: return B576000;
#endif
#ifdef B921600
        case 921600: return B921600;
#endif
#ifdef B1000000
        case 1000000: return B1000000;
#endif
#ifdef B1152000
        case 1152000: return B1152000;
#endif
#ifdef B1500000
        case 1500000: return B1500000;
#endif
#ifdef B2000000
        case 2000000: return B2000000;
#endif
#ifdef B2500000
        case 2500000: return B2500000;
#endif
#ifdef B3000000
        case 3000000: return B3000000;
#endif
#ifdef B3500000
        case 3500000: return B3500000;
#endif
#ifdef B4000000
        case 4000000: return B4000000;
#endif
        default:
            return -1;
        }
    }


private:

    Device::Received _readed;
    core::Synchronizer &_synchronizer;
    int _device;
    speed_t _baudRate;    // Baudrate
    Device::Parity _parity;           // Parity
    Device::ByteSize _byteSize;       // Size of the bytes
    Device::StopBits _stopBits;       // Stop Bits
    Device::FlowControl _flowControl; // Flow Control

    typedef std::list<Send *> Pendings;
    Pendings _pendings;
};

Device::Device():
    _private(NULL)
{

}

Device::~Device()
{

}

int Device::_open(const char *path,
                  const core::Variant &settings,
                  const Received &readed,
                  core::Synchronizer &synchronizer)
{
    if (_private != NULL)
        return -1;
    _private = new Private(readed, synchronizer);
    int error = _private->open(path, settings);
    if (error != 0) {
        delete _private;
        _private = NULL;
        return error;
    }
    return 0;
}

int Device::_write(const uint8_t *packet, const devices::Device::Size &size)
{
    if (_private == NULL)
        return -1;
    return _private->write(packet, size);
}

void Device::close()
{
    if (_private == NULL)
        return;
    _private->close();
    delete _private;
    _private = NULL;
}

} // namespace serial
} // namespace devices
} // namespace iot


