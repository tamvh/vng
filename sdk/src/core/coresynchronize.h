#ifndef CORESYNCHRONIZE_H
#define CORESYNCHRONIZE_H
#include <sdkdefs.h>
#include <map>

namespace iot {
namespace core {
class Executer;
class SDKSHARED_EXPORT Synchronizer
{
public:
    Synchronizer(int events, int timeout);
    virtual ~Synchronizer();
public:
    class Callback
    {
    public:
        Callback() {}
        virtual void read() = 0;
        virtual bool wantRead() const = 0;
        virtual void write() = 0;
        virtual bool wantWrite() const = 0;
        virtual void timeout() = 0;
    };

    int append(int descriptor, Callback *callback);
    int update(int descriptor);
    void remove(int descriptor);
public:
    int start(Executer &executer);
    int skip();
    void stop();
private:
    class Private;
    Private *_private;
};

} // namespace Core
} // namespace iot

#endif // CORESYNCHRONIZE_H
