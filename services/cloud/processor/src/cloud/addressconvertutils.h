#ifndef ADDRESSCONVERTUTILS_H
#define ADDRESSCONVERTUTILS_H

#include <stdint.h>
#include <string>

class AddressConvertUtils
{
public:
    AddressConvertUtils();

    static bool toHex(char character, uint8_t &hex);
    static bool toBLEAddressId(const std::string &string, uint8_t *address);
};

#endif // ADDRESSCONVERTUTILS_H
