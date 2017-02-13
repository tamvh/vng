#ifndef NETAPPLICATION_H
#define NETAPPLICATION_H
#include <sdkdefs.h>
#include <core/coreapplication.h>
#include <list>
namespace iot {
namespace net {

class Channel;
class SDKSHARED_EXPORT Application: public core::Application
{
public:
    Application(const std::string &config);
    virtual ~Application();
private:
    virtual int initialize(core::Synchronizer& synchronizer,
                           const core::Variant &settings);
    virtual void release();
private:
    virtual int initialize(core::Synchronizer& synchronizer,
                           const core::Variant &settings,
                           Channel &channel) = 0;
    virtual void release(Channel &channel) = 0;

public:
    static Application &instance();
private:
    class Private;
    Private *_private;
    friend class Private;
};
} // namespace net
} // namespace iot
#endif // NETAPPLICATION_H
