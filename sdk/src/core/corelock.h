#ifndef CORESLOCK_H
#define CORESLOCK_H
#include <sdkdefs.h>
namespace iot {
namespace core {
class SDKSHARED_EXPORT Locker
{
public:
    Locker();
    ~Locker();
    void lock();
    void unlock();
private:
    class Private;
    Private *_private;
};

} // namespace Core
} // namespace iot
#endif // CORESLOCK_H
