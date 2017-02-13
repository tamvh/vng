#ifndef TIMERFD_H
#define TIMERFD_H

#include <unistd.h>
#include <sys/timerfd.h>
#include <functional>
#include <core/coresynchronize.h>

class TimerFD : public iot::core::Synchronizer::Callback
{
public:
    typedef std::function<void(void) > VoidCallback;

    TimerFD()
    {
        _timerfd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK);
    }

    ~TimerFD()
    {
        ::close(_timerfd);
    }

    int fd()
    {
        return _timerfd;
    }

    void setTime(int timeout_ms, bool periodic = false)
    {
        struct itimerspec newVal;
        newVal.it_interval.tv_sec = 0;
        if (periodic) {
            newVal.it_interval.tv_nsec = 1e6 * timeout_ms;
        } else {
            newVal.it_interval.tv_nsec = 0;
        }
        newVal.it_value.tv_sec = 0;
        newVal.it_value.tv_nsec = 1e6 * timeout_ms;
        timerfd_settime(_timerfd, 0, &newVal, NULL);
    }

    void setCallback(VoidCallback cb)
    {
        _cb = cb;
    }

public:

    void read()
    {
        uint64_t v;
        ssize_t r;
        while ((r = ::read(_timerfd, &v, sizeof (v))) > 0);
        if (_cb)
            _cb();
    }

    bool wantRead() const
    {
        return true;
    }

    void write()
    {
    }

    bool wantWrite() const
    {
        return false;
    }

    void timeout()
    {
    }
protected:
    int _timerfd;
    VoidCallback _cb;
};

#endif /* TIMERFD_H */

