#ifndef COREAPPLICATION_H
#define COREAPPLICATION_H
#include <sdkdefs.h>
#include <string>
#include <map>
using namespace std;
namespace iot {
namespace core {
class Variant;
class Synchronizer;
class Executer;
class Command;
class SDKSHARED_EXPORT Application
{
public:
    Application(const std::string &config);

    virtual ~Application();

    int exec(bool terminate = false);

    int schedule(Command *command);

    void exit(int code);
private:
    virtual int initialize(core::Synchronizer& synchronizer,
                           const core::Variant &settings) = 0;
    virtual void release() = 0;
public:
    static Application *instance();
private:
    class Private;
    Private *_private;
    friend class Private;
};

} // namespace Core
} // namespace iot

#endif // COREAPPLICATION_H
