#include <core/coresynchronize.h>
#include <core/coreapplication.h>
#include <core/corecommand.h>
#include <core/coreexecute.h>
#include <unistd.h>
#include <stdio.h>

#ifdef WITH_POSIX
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#endif

#ifdef USE_EPOLL
#include <sys/epoll.h>
#include <sys/eventfd.h>
#elif defined USE_KQUEUE
#include <sys/event.h>
#endif

#include <map>

namespace iot {
namespace core {

class Synchronizer::Private: public Synchronizer::Callback
{
    class Synchronize: public Command
    {
    public:
        Synchronize(Private &priv):
            _private(priv),
            _finished(false) {

        }

        inline void finish() {
            _finished = true;
        }

    private:
        virtual void execute() {
            if (_finished) {
                delete this;
                return;
            }
            _private.synchronize();
        }

    private:
        Private &_private;
        bool _finished;
    };
public:
#ifdef USE_EPOLL
    Private(int events, int timeout):
        _event(NULL),
        _skipper(-1),
        _handle(-1),
        _events(events),
        _timeout(timeout) {

    }

    virtual ~Private() {
    }
    virtual void read() {
        eventfd_t event;
        eventfd_read(_skipper, &event);
    }

    virtual bool wantRead() const {
        return true;
    }

    virtual void write() {

    }

    virtual bool wantWrite() const {
        return false;
    }

    virtual void timeout() {

    }

    int start(Executer &executer) {
        _handle = epoll_create(_events);
        if (_handle < 0)
            return errno;


        _event = (epoll_event *)calloc(_events, sizeof(struct epoll_event));
        if (_event == NULL) {
            close(_handle);
            _handle = -1;
            return -1;
        }
        _skipper = eventfd(0, 0);
        int error;
        if (_skipper == -1) {
            error = errno;
            free(_event);
            _event = NULL;
            close(_handle);
            _handle = -1;
            return error;
        }

        error = append(_skipper, this);
        if (error != 0) {
            close(_skipper);
            _skipper = -1;
            free(_event);
            _event = NULL;
            close(_handle);
            _handle = -1;
            return error;
        }
        _synchronize = new Synchronize(*this);
        executer.schedule(_synchronize, true);
        return 0;
    }

    int skip() {
        int bytes =  eventfd_write(_skipper, 1);
        return bytes;
    }

    void stop() {
        if (_synchronize == NULL)
            return;
        close(_skipper);
        _skipper = -1;
        _synchronize->finish();
        _synchronize = NULL;

        close(_handle);
        _handle = -1;
        free(_event);
        _event = NULL;
    }


    int append(int descriptor, Synchronizer::Callback *callback) {
        _callbacks[descriptor] = callback;
        struct epoll_event event;
        event.data.ptr = callback;
        event.events = 0;
        if (callback->wantRead())
            event.events |= EPOLLIN;

        if (callback->wantWrite())
            event.events |= EPOLLOUT;

        if (event.events != 0)
            event.events |= EPOLLHUP | EPOLLERR;

        if (epoll_ctl(_handle, EPOLL_CTL_ADD, descriptor, &event) < 0) {
            _callbacks.erase(descriptor);
            printf("*Synchronizer* append to epoll failed\n");
            return errno;
        }
        return 0;
    }

    int update(int descriptor) {

        Callbacks::iterator found = _callbacks.find(descriptor);
        if (found == _callbacks.end())
            return -1;

        Synchronizer::Callback *callback = found->second;
        struct epoll_event event;
        event.data.ptr = callback;
        event.events = 0;
        if (callback->wantRead())
            event.events |= EPOLLIN;

        if (callback->wantWrite())
            event.events |= EPOLLOUT;

        if (event.events != 0)
            event.events |= EPOLLHUP | EPOLLERR;

        if (epoll_ctl(_handle, EPOLL_CTL_MOD, descriptor, &event) < 0) {

            _callbacks.erase(descriptor);
            return errno;
        }
        return 0;
    }

    void remove(int descriptor) {
        Callbacks::iterator iterator = _callbacks.find(descriptor);
        if (iterator == _callbacks.end())
            return;
        epoll_ctl(_handle, EPOLL_CTL_DEL, descriptor, NULL);
        _callbacks.erase(iterator);
    }

    void synchronize() {
        int result = epoll_wait(_handle, _event, _events, _timeout);
        if (result <= 0) {
            for(int index = 0; index < result; index++) {
                struct epoll_event *event = _event + index;
                Synchronizer::Callback *callback;
                callback = (Synchronizer::Callback *) event->data.ptr;
                callback->timeout();
            }
            return;
        }

        for(int index = 0; index < result; index++) {
            struct epoll_event *event = _event + index;
            Synchronizer::Callback *callback;
            callback = (Synchronizer::Callback *) event->data.ptr;
            if ((event->events & EPOLLIN) == EPOLLIN)
                callback->read();
            if ((event->events & EPOLLOUT) == EPOLLOUT)
                callback->write();
        }
    }

    inline const int &timeout() const {
        return _timeout;
    }
#endif
#ifdef USE_KQUEUE
    Private(int events, int timeout):
        _event(NULL),
        _uses(0),
        _handle(0),
        _events(events),
        _timeout(timeout) {

    }
    virtual void read() {

    }

    virtual bool reading() const {
        return true;
    }

    virtual void write() {

    }

    virtual bool writing() const {
        return false;
    }

    int start(Executer &executer) {

        _handle = kqueue();
        if (_handle < 0)
            return errno;

        _event = (struct kevent *)calloc(_events, sizeof(struct kevent));
        if (_event == NULL) {
            close(_handle);
            _handle = 0;
            return -1;
        }
        _fire = (struct kevent *)calloc(_events, sizeof(struct kevent));
        if (_fire == NULL) {
            free(_event);
            _event = NULL;
            close(_handle);
            _handle = 0;
            return -1;
        }
        _uses = 0;
        _synchronize = new Synchronize(*this);
        executer.schedule(_synchronize, true);
        return 0;
    }

    void stop() {
        if (_synchronize == NULL)
            return;

        _synchronize->finish();
        _synchronize = NULL;

        close(_handle);
        _handle = 0;
        free(_event);
        _event = NULL;
    }


    int append(int descriptor,
               Synchronizer::Reader &reader,
               Synchronizer::Writer &writer) {
        if (_pollers.find(descriptor) != _pollers.end())
            return -1;
        Poller *poller = new Poller(descriptor, reader, writer, _uses++);
        _pollers[descriptor] = poller;
        short filter = 0;
        if (reader.reading())
            filter |= EVFILT_READ;

        if (writer.writing())
            filter |= EVFILT_WRITE;
        struct kevent *event = _event + poller->position();
        EV_SET(event,
               poller->descriptor(),
               filter,
               EV_ADD | EV_ONESHOT,
               0,
               0,
               poller);
        return 0;
    }


    int update(int descriptor) {

        Pollers::iterator end = _pollers.end();
        Pollers::iterator found = _pollers.find(descriptor);
        if (found == end)
            return -1;
        Poller *poller = found->second;
        short filter = 0;
        if (poller->reading())
            filter |= EVFILT_READ;

        if (poller->writing())
            filter |= EVFILT_WRITE;


        return 0;
    }

    void remove(int socket) {
        Pollers::iterator iterator = _pollers.find(socket);
        if (iterator == _pollers.end())
            return;

        Poller *poller = iterator->second;
        _pollers.erase(iterator);
        delete poller;
    }

    void synchronize() {
        struct timespec timeout;
        timeout.tv_sec = _timeout / 1000;
        timeout.tv_nsec = (_timeout - timeout.tv_sec * 1000) ;
        int fires = kevent(_handle, _event, _uses, _fire, _events, &timeout);
        if (fires == -1)
            return;

        for(int position = 0; position < fires; position++) {
            struct kevent *event = _fire + position;
            Poller *poller = (Poller *) event->udata;
            if ((event->filter & EVFILT_READ) == EVFILT_READ)
                poller->read();
            if ((event->filter & EVFILT_WRITE) == EVFILT_WRITE)
                poller->write();
        }
    }

    int skip() {
        return 0;
    }
    inline const int &timeout() const {
        return _timeout;
    }

#endif
private:
#ifdef USE_EPOLL
    struct epoll_event *_event;
    int _skipper;
#elif defined USE_KQUEUE
    struct kevent *_event;
    struct kevent *_fire;
    int _uses;
#endif

    int _handle;
    int _events;
    int _timeout;
    typedef std::map<int, Callback *> Callbacks;
    Callbacks _callbacks;
    Synchronize *_synchronize;
};

/**
 * @brief Synchronizer::Synchronizer
 */
Synchronizer::Synchronizer(int events, int timeout):
    _private(new Private(events, timeout))
{

}

Synchronizer::~Synchronizer()
{
    delete _private;
}


int Synchronizer::append(int descriptor, Callback *maintainer)
{
    return _private->append(descriptor, maintainer);
}

int Synchronizer::update(int descriptor)
{
    return _private->update(descriptor);
}

void Synchronizer::remove(int descriptor)
{
    _private->remove(descriptor);
}

int Synchronizer::start(Executer &executer)
{
    int error = _private->start(executer);
    if (error != 0) {
        delete _private;
        _private = NULL;
        return error;
    }
    return 0;
}

int Synchronizer::skip()
{
    return _private->skip();
}

void Synchronizer::stop()
{
    _private->stop();
}


} // namespace Core
} // namespace iot

