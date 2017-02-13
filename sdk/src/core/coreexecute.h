#ifndef COREEXECUTE_H
#define COREEXECUTE_H
#include <sdkdefs.h>
namespace iot {
namespace core {
class Processor;
class Command;
class SDKSHARED_EXPORT Executer
{
public:
    Executer();
    virtual ~Executer();
public:
    int schedule(Command *command, bool primary);
    void setPrimary(Command *command);
    int exec();
    void exit(int code);
private:
    class Private;
    Private *_private;
};

} // namespace Core
} // namespace iot
#endif // COREEXECUTE_H
