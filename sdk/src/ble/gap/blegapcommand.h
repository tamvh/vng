#ifndef BLEGAPCOMMAND_H
#define BLEGAPCOMMAND_H
#include <core/corecommand.h>
namespace iot {
namespace ble {
namespace gap {

class Command: public core::Command
{
public:
    Command();
};

} // namespace gap
} // namespace ble
} // namespace iot

#endif // BLEGAPCOMMAND_H
