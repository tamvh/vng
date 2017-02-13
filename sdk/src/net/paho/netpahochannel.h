#ifndef NETPAHOCHNNEL_H
#define NETPAHOCHNNEL_H
#include <net/netchannel.h>
#include <string>
namespace iot {
namespace net {
namespace paho {
class SDKSHARED_EXPORT Channel: public net::Channel
{
public:
    Channel(const net::Address &address, const core::Variant &settings);
    virtual ~Channel();
private:
    virtual int intialize(core::Synchronizer &synchronizer,
                          const net::Address &address,
                          const core::Variant &settings,
                          const net::Channel::Receiver &receiver);

    virtual int subscribe(const net::Channel::Topic &topic);

    virtual int publish(const net::Channel::Topic &topic,
                        const net::Channel::Payload *payload,
                        const net::Channel::Size &size,
                        net::Channel::Token &token);

    virtual int unsubscribe(const net::Channel::Topic &topic);

    virtual void release();
private:
    class Private;
    Private *_private;
    friend class Private;
};

} // namespace paho
} // namespace net
} // namespace iot

#endif // NETPAHOCHNNEL_H
