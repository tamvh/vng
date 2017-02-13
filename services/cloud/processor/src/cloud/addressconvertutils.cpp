#include "addressconvertutils.h"
#include <ble/bledefs.h>
#include <utils/utilsprint.h>

AddressConvertUtils::AddressConvertUtils()
{

}

bool AddressConvertUtils::toHex(char character, uint8_t &hex) {
    if ('0' <= character && character <= '9') {
        hex = character - '0';
        return true;
    }
    if ('A' <= character && character <= 'F') {
        hex = character - 'A' + 10;
        return true;
    }
    if ('a' <= character && character <= 'a') {
        hex = character - 'a' + 10;
        return true;
    }
    PRINTF("toHex failed\r\n");
    return false;
}

bool AddressConvertUtils::toBLEAddressId(const std::string &string, uint8_t *address) {

    std::string::size_type length = string.length();
    if (length != 17)
        return false;

    uint8_t position = 0;
    for(std::string::size_type index = 0; index < length; position += 1) {
        uint8_t first;
        if (!toHex(string[index], first))
            return false;

        uint8_t second;
        if (!toHex(string[index + 1], second))
            return false;
        address[position] = (first << 4) + second;
        if (position < BLE_ADDRESS_SIZE - 1) {
            if (string[index + 2] != ':')
                return false;
        }
        index += 3;
    }

    return position == BLE_ADDRESS_SIZE;
}
