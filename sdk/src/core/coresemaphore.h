#ifndef CORESEMAPHORE_H
#define CORESEMAPHORE_H
#include <sdkdefs.h>
namespace iot {
namespace core {
class SDKSHARED_EXPORT Semaphore
{
public:
    Semaphore();
    ~Semaphore();
    int wait();
    int signal();
private:
    class Private;
    Private *_private;
};
} // namespace Core
} // namespace iot
#endif // CORESEMAPHORE_H
