#ifndef CORECOMMAND_H
#define CORECOMMAND_H
#include <sdkdefs.h>
namespace iot {
namespace core {
class Executer;
class SDKSHARED_EXPORT Command
{
public:
    virtual ~Command() {}
    virtual void execute() = 0;
};
} // namespace Core
} // namespace iot
#endif // CORECOMMAND_H
