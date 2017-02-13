#ifndef NETCHANNEL_H
#define NETCHANNEL_H
#include <sdkdefs.h>
#include <core/corecallback.h>
#include <string>
#include <vector>
#include <stdint.h>
namespace iot {
namespace comm {
class Message;
}
namespace core {
class Variant;
class Synchronizer;
}

namespace net {
class Address;
class SDKSHARED_EXPORT Channel
{
public:
    typedef int Token;
    typedef std::string Topic;
    typedef uint8_t Payload;
    typedef std::vector<Payload> Packet;
    typedef uint32_t Size;
    typedef core::Callback<void(const Topic &topic,
                                const Packet &payload)> Subscriber;

    Channel(const Address &address, const core::Variant &settings);

    virtual ~Channel();

    int intialize(core::Synchronizer &synchronizer);

    int subscribe(const Topic &topic,
                  const Subscriber &subscriber,
                  Token &token);


    int publish(const Topic &topic,
                const comm::Message &message,
                Token &token);

    void unsubscribe(const Topic &topic, const Token &token);

protected:
    typedef core::Callback<void(const Topic &topic,
                                const Packet &payload)> Receiver;
private:
    virtual int intialize(core::Synchronizer &synchronizer,
                          const Address &address,
                          const core::Variant &settings,
                          const Receiver &receiver) = 0;

    virtual int subscribe(const Topic &topic) = 0;

    virtual int unsubscribe(const Topic &topic) = 0;
public:
    virtual int publish(const Topic &topic,
                        const Payload *payload,
                        const Size &size,
                        Token &token) = 0;

    virtual void release() = 0;
private:
    class Private;
    Private *_private;
};

} // namespace net
} // namespace iot

#endif // NETCHANNEL_H
