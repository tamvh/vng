#ifndef NETSUBSCRIBE_H
#define NETSUBSCRIBE_H
#include <sdkdefs.h>
#include <string>
#include <stdint.h>
#include <comm/commmessage.h>
namespace iot {
namespace core {
class Executer;
class Command;
}
namespace net {
class Channel;
class SDKSHARED_EXPORT Subscriber
{
public:
    typedef std::string Topic;
    typedef uint16_t Size;

    Subscriber(Channel &channel);

    virtual ~Subscriber();

    int subscribe(const Topic &topic);

    Channel &channel();

    void unsubcrible(const Topic &topic);

private:
    virtual core::Command *parse(const comm::Message::Id &id,
                                 const uint8_t *payload,
                                 const Size &size);
private:
    class Private;
    Private *_private;
    friend class Private;
};

} // namespace net
} // namespace iot
#endif // COMMNETSUBSCRIBE_H
