#include <core/corelock.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
namespace iot {
namespace core {
class Locker::Private
{
public:
    Private() {

        int error = pthread_mutex_init(&_mutex, NULL);
        if (error != 0) {
            fprintf(stderr, "initialize lock failed with error: %d", error);
            exit(error);
        }
    }
    ~Private() {
        pthread_mutex_destroy(&_mutex);
    }

    void lock() {
        int error = pthread_mutex_lock(&_mutex);
        if (error == 0)
            return;
        fprintf(stderr, "lock failed with error: %d", error);
        exit(error);
    }

    void unlock() {
        int error = pthread_mutex_unlock(&_mutex);
        if (error == 0)
            return;
        fprintf(stderr, "unlock failed with error: %d", error);
        exit(error);
    }

private:
    pthread_mutex_t _mutex;
};

Locker::Locker():
    _private(new Private())
{

}

Locker::~Locker()
{
    delete _private;
}


void Locker::lock()
{
    _private->lock();
}

void Locker::unlock()
{
    _private->unlock();
}


} // Utils
} // IoT
