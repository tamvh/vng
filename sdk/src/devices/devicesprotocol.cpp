#include <devices/devicesprotocol.h>
#include <utils/utilscrc16.h>
#include <utils/utilsprint.h>
#include <map>
#include <stdio.h>
namespace iot {
namespace devices {


Protocol::~Protocol()
{

}

namespace slbp {

class Protocol::Private
{
public:
    Private():
        _newline(false),
        _value(0),
        _position(0) {
    }

    virtual ~Private() {

    }

    int encode(const uint8_t *packet,
               const devices::Protocol::Size &size,
               devices::Protocol::Packet &encoded) {

        devices::Protocol::Size encodedSize = size * 2 +
                                              sizeof(utils::CRC16::CRC) * 2
                                              + 2;
        PRINTF("*slbp* encoded size: %d", encodedSize);
        utils::CRC16::CRC crc = utils::CRC16::calculate(packet, size);
        encoded.resize(encodedSize);
        uint8_t *output = encoded.data();
        devices::Protocol::Size position = 0;
        char text[16];
        for(Size index = 0; index < size; index++) {
            sprintf(text, "%02X", packet[index]);
            output[position++] = text[0];
            output[position++] = text[1];
        }
        const uint8_t *crcs = (const uint8_t *)&crc;
        for(Size index = 0; index < sizeof(utils::CRC16::CRC); index++) {
            sprintf(text, "%02X", crcs[index]);
            output[position++] = text[0];
            output[position++] = text[1];
        }
        output[position++] = 0x0A;
        output[position++] = 0x0D;
        return 0;
    }

    int decode(const uint8_t *packet,
               const devices::Protocol::Size &size,
               const devices::Protocol::Decoded &decoded) {

        for(devices::Protocol::Size index = 0; index < size; index++) {
            uint8_t character = packet[index];
            char value;
            printf("%02X", character);
            if (character == '\r') {
                if (!_newline)
                    _newline = true;
                else
                    reset();
                continue;
            }

            if (character == '\n') {
                if (_newline && _position == 0)
                    finish(decoded);
                reset();
                continue;
            }

            if ('0' <= character && character <= '9') {
                value = character - '0';
            } else if ('A' <= character && character <= 'F') {
                value = character - 'A' + 0x0A;
            } else {
                reset();
                continue;
            }

            _value += value << (4 * (1 - _position));
            _position = (_position + 1) % 2;
            if (_position == 0) {
                _packet.push_back(_value);
                _value = 0;
            }
        }
        return 0;
    }

    void finish(const devices::Protocol::Decoded &decoded) {
        const uint8_t *packet = _packet.data();
        devices::Protocol::Size rawSize = _packet.size();
        if (rawSize < sizeof(utils::CRC16::CRC))
            return;

        devices::Protocol::Size size = _packet.size() -
                                       sizeof(utils::CRC16::CRC);


        utils::CRC16::CRC lowByte = packet[size];
        utils::CRC16::CRC hiByte = packet[size + 1];

        utils::CRC16::CRC sourceCrc = (lowByte << 8) + hiByte;

        utils::CRC16::CRC calculatedCrc = utils::CRC16::calculate(packet,
                                                                  size);
        if (sourceCrc != calculatedCrc) {
            printf("*slbp* checksum not valided\r\n"
                   "\t\t\tsource: 0x%04X\r\n,"
                   "\t\tcalculated: 0x%04X\r\n",
                   sourceCrc,
                   calculatedCrc);
            return;
        }

        decoded.call(packet, size);

    }

    void reset() {
        _packet.clear();
        _position = 0;
        _value = 0;
        _newline = false;
    }
private:
    bool _newline;
    uint8_t _value;
    devices::Protocol::Size _position;
    devices::Protocol::Packet _packet;
};

Protocol::Protocol():
    _private(new Private)
{

}

Protocol::~Protocol()
{
    delete _private;
}


int Protocol::encode(const uint8_t *packet,
                     const devices::Protocol::Size &size,
                     devices::Protocol::Packet &encoded)
{
    return _private->encode(packet, size, encoded);
}

int Protocol::decode(const uint8_t *packet,
                     const devices::Protocol::Size &size,
                     const devices::Protocol::Decoded &decoded)
{
    return _private->decode(packet, size, decoded);
}

void Protocol::reset()
{
    _private->reset();
}

} // namespace slbp

namespace slip {
#define SLIP_END             0xC0    /* indicates end of packet */
#define SLIP_ESC             0xDB    /* indicates byte stuffing */
#define SLIP_ESC_END         0xDC    /* ESC ESC_END means END data byte */
#define SLIP_ESC_ESC         0xDD    /* ESC ESC_ESC means ESC data byte */
#define SLIP_ESC_XON  0xDE /* ESC SLIP_ESC_XON means XON control byte */
#define SLIP_ESC_XOFF 0xDF /* ESC SLIP_ESC_XON means XOFF control byte */
#define XON 0x11 /* indicates XON charater */
#define XOFF 0x13 /* indicates XOFF charater */

class Protocol::Private
{
public:
    typedef devices::Protocol::Size Size;
    typedef devices::Protocol::Packet Packet;
    typedef utils::CRC16::CRC CRC;
    Private():
        _escaping(false) {
        _decoded.resize(32);
        _decoded.clear();
    }

    virtual ~Private() {

    }

    int encode(const uint8_t *packet, const Size &size, Packet &encoded) {
        utils::CRC16::CRC crc = utils::CRC16::calculate(packet, size);
        Size mainSize = Private::encodedSize(packet, size);
        Size crcSize = Private::encodedSize((const uint8_t *) &crc,
                                            sizeof(CRC));

        encoded.resize(mainSize + crcSize + 1);

        uint8_t *values = encoded.data();


        encode(packet, size, values, mainSize);

        encode((uint8_t *)&crc, sizeof(CRC), values + mainSize, crcSize);
        values[mainSize + crcSize] = SLIP_END;
        return 0;
    }

    int decode(const uint8_t *packet,
               const devices::Protocol::Size &size,
               const devices::Protocol::Decoded &decoded) {
        for(Size index = 0; index < size; index ++) {
            uint8_t value = packet[index];
            if (_escaping) {

                if (value == SLIP_ESC_END)
                    _decoded.push_back(SLIP_END);
                else if (value == SLIP_ESC_ESC)
                    _decoded.push_back(SLIP_ESC);
                else if (value == SLIP_ESC_XON)
                    _decoded.push_back(XON);
                else if (value == SLIP_ESC_XOFF)
                    _decoded.push_back(XOFF);
                else
                    _decoded.push_back(value);

                _escaping = false;
                continue;
            }
            if (value == SLIP_END) {
                finish(decoded);
            } else if (value == SLIP_ESC) {
                _escaping = true;
            } else {
                _decoded.push_back(value);
            }
        }
        return 0;
    }

    void reset() {
        _escaping = false;
        _decoded.clear();
    }
private:
    static Size encodedSize(const uint8_t *payload,
                            const Size &size) {

        Size position = 0;
        for(Size index = 0; index < size; index++) {
            uint8_t value = payload[index];
            if (value == SLIP_END) {
                position += 2;
            } else if (value == SLIP_ESC){
                position += 2;
            } else if (value == XON){
                position += 2;
            } else if (value == XOFF){
                position += 2;
            } else {
                position += 1;
            }
        }
        return position;
    }

    void encode(const uint8_t *packet,
                const Size &size,
                uint8_t *encoded,
                Size &encodedSize) {

        encodedSize = 0;
        for(Size index = 0; index < size; index++) {
            uint8_t value = packet[index];
            if (value == SLIP_END) {
                encoded[encodedSize++] = SLIP_ESC;
                encoded[encodedSize++] = SLIP_ESC_END;
            } else if (value == SLIP_ESC){
                encoded[encodedSize++] = SLIP_ESC;
                encoded[encodedSize++] = SLIP_ESC_ESC;
            } else if (value == XON){
                encoded[encodedSize++] = SLIP_ESC;
                encoded[encodedSize++] = SLIP_ESC_XON;
            } else if (value == XOFF){
                encoded[encodedSize++] = SLIP_ESC;
                encoded[encodedSize++] = SLIP_ESC_XOFF;
            } else {
                encoded[encodedSize++] = value;
            }
        }
    }
    void finish(const devices::Protocol::Decoded &decoded) {

        Size size = _decoded.size();
        if (size < sizeof(CRC))
            return;

        const uint8_t *values = _decoded.data();

        Size mainSize = size - sizeof(CRC);
        // Checksum
        const CRC *courceCrc = (const CRC *)(values + mainSize);
        CRC calculatedCrc = utils::CRC16::calculate(values, mainSize);
        if (*courceCrc == calculatedCrc) {
            PRINTF("*slip* decoded\r\n");
            decoded.call(values, mainSize);
        } else {
            PRINTF("*slip* decoded crc not match.\r\n"
                   "\t\t\t\t\tsize: %d\r\n"
                   "\t\t\t\t\tsource: 0x%04X\r\n"
                   "\t\t\t\tcalculated: 0x%04X\r\n",
                   mainSize,
                   *courceCrc,
                   calculatedCrc);
        }
        _decoded.clear();

    }
private:
    bool _escaping;
    Packet _decoded;
};

Protocol::Protocol():
    _private(new Private)
{

}

Protocol::~Protocol()
{
    delete _private;
}


int Protocol::encode(const uint8_t *packet,
                     const devices::Protocol::Size &size,
                     devices::Protocol::Packet &encoded)
{
    return _private->encode(packet, size, encoded);
}

int Protocol::decode(const uint8_t *packet,
                     const devices::Protocol::Size &size,
                     const devices::Protocol::Decoded &decoded)
{
    return _private->decode(packet, size, decoded);
}

void Protocol::reset()
{
    _private->reset();
}

} // namespace slip
} // namespace devices
} // namespace iot

