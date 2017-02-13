#include <core/coresemaphore.h>
#include <semaphore.h>
#include <stdlib.h>

namespace iot {
namespace core {
class Semaphore::Private
{
public:
    Private() {
        int error = sem_init(&_semaphore, 0, 0);
        if (error != 0)
            exit(error);
    }
    ~Private() {
        sem_destroy(&_semaphore);
    }

    int wait() {
        return sem_wait(&_semaphore);
    }

    int signal() {
        return sem_post(&_semaphore);
    }

private:
    sem_t _semaphore;
};

Semaphore::Semaphore():
    _private(new Private())
{

}

Semaphore::~Semaphore()
{
    delete _private;
}

int Semaphore::wait()
{
    return _private->wait();
}

int Semaphore::signal()
{
    return _private->signal();
}

} // namespace Core
} // namespace iot
