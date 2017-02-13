#include <net/netsubscribe.h>
#include <net/netchannel.h>
#include <core/corecommand.h>
#include <core/coreapplication.h>
#include <map>
#include <stdio.h>

namespace iot {
namespace net {
struct Event
{
    typedef comm::Message::Id Id;
    typedef uint16_t Size;
    Id id;
    Size size;
    uint8_t payload[0];
} __attribute__((packed));

class Subscriber::Private
{
public:
    Private(Channel &channel, Subscriber &subscriber):
        _channel(channel),
        _subscriber(subscriber) {

    }

    void received(const Channel::Topic &topic,
                  const Channel::Packet &packet) {
        printf("packet.size(): %d\n", packet.size());
        if (sizeof(Event) > packet.size())
            return;

        const Event *event = (const Event *) packet.data();
//        if (event->size + sizeof(struct Event) > packet.size())
//            return;



        core::Command *command = _subscriber.parse(event->id,
                                                   event->payload,
                                                   event->size);
        if (command == NULL)
            return;

        if (core::Application::instance()->schedule(command) == 0)
            return;
        delete command;
    }

    int subcrible(const Subscriber::Topic &topic) {
        if (_topics.find(topic) != _topics.end())
            return -1;
        Channel::Token token;
        Channel::Subscriber subscriber(this,&Private::received);
        int error = _channel.subscribe(topic, subscriber, token);
        if (error != 0)
            return error;
        _topics[topic] = token;
        return 0;
    }

    inline Channel &channel() {
        return _channel;
    }

    void unsubcrible(const Subscriber::Topic &topic) {
        Topics::iterator found = _topics.find(topic);
        if (found == _topics.end())
            return;
        _channel.unsubscribe(topic, found->second);
        _topics.erase(topic);
    }

private:
    Channel &_channel;
    Subscriber &_subscriber;
    typedef std::map<Subscriber::Topic, Channel::Token> Topics;
    Topics _topics;
};
Subscriber::Subscriber(Channel &channel):
    _private(new Private(channel, *this))
{

}

Subscriber::~Subscriber()
{
    delete _private;
}

int Subscriber::subscribe(const Topic &topic)
{
    return _private->subcrible(topic);
}

Channel &Subscriber::channel()
{
    return _private->channel();
}

void Subscriber::unsubcrible(const Topic &topic)
{
    _private->unsubcrible(topic);
}

core::Command *Subscriber::parse(const comm::Message::Id &/*id*/,
                                 const uint8_t */*payload*/,
                                 const Size &/*size*/)
{
    return NULL;
}
} // namespace net
} // namespace iot
