#include <net/netchannel.h>
#include <net/netaddress.h>
#include <comm/commmessage.h>
#include <core/corelock.h>
#include <core/corevariant.h>
#include <stdio.h>
namespace iot {
namespace net {

class Channel::Private
{
public:
    class Topic
    {
    public:
        Topic():
            _token(0) {

        }

        virtual ~Topic() {

        }

        void subscribe(const Channel::Subscriber &subscriber,
                       Channel::Token &token) {
            _subscribers[_token ++] = subscriber;
            token = _token;
        }

        inline bool emptied() const {
            return _subscribers.size() == 0;
        }

        void receive(const Channel::Topic &topic,
                     const Channel::Packet &payload) {

            Subscribers::iterator end = _subscribers.end();
            for(Subscribers::iterator iterator = _subscribers.begin();
                iterator != end;
                iterator++) {
                const Channel::Subscriber &subscriber = iterator->second;
                subscriber.call(topic, payload);
            }
        }

        void unsubscribe(const Channel::Token &token) {
            _subscribers.erase(token);
        }


    private:
        Channel::Token _token;
        typedef std::map<Channel::Token, Channel::Subscriber> Subscribers;
        Subscribers _subscribers;
    };

    typedef std::map<Channel::Topic, Topic *> Topics;
    Private(const Address &address, const core::Variant &settings):
        _address(address),
        _settings(settings) {

    }

    inline const Address &address() const {
        return _address;
    }

    void subscrible(const Channel::Topic &id,
                    const Channel::Subscriber &subscriber,
                    Channel::Token &token) {

        Topics::iterator found = _topics.find(id);

        Topic *topic;
        if (found != _topics.end()) {
            topic = found->second;
        } else {
            topic = new Topic;
            _topics[id]  = topic;
        }
        topic->subscribe(subscriber, token);
    }

    inline bool subscribed(const Channel::Topic &topic) const {
        return _topics.find(topic) != _topics.end();
    }

    void received(const Channel::Topic &topicId,
                  const Channel::Packet &payload) {

        Topics::iterator found = _topics.find(topicId);
        if (found == _topics.end())
            return;

        Topic *topic = found->second;
        topic->receive(topicId, payload);
    }

    void unsubscrible(const Channel::Topic &id, const Channel::Token &token) {
        Topics::iterator found = _topics.find(id);
        if (found == _topics.end())
            return;
        Topic *topic = found->second;
        topic->unsubscribe(token);
        if (!topic->emptied())
            return;
        _topics.erase(id);
        delete topic;
    }

    inline const core::Variant &settings() const {
        return _settings;
    }

private:
    Address _address;
    core::Variant _settings;
    Topics _topics;
};

Channel::Channel(const Address &address, const core::Variant &settings):
    _private(new Private(address, settings))
{

}

Channel::~Channel()
{
    delete _private;
}

int Channel::intialize(core::Synchronizer &synchronizer)
{
    return intialize(synchronizer,
                     _private->address(),
                     _private->settings(),
                     Receiver(_private, &Private::received));
}

int Channel::publish(const Topic &topic,
                     const comm::Message &message,
                     Token &token)
{
    comm::Message::Packet packet;
    message.fill(packet);
    return publish(topic, packet.data(), packet.size(), token);
}

int Channel::subscribe(const Topic &topic,
                       const Subscriber &subscriber,
                       Token &token)
{
    int error;
    bool subscribed = _private->subscribed(topic);
    if (subscribed != true) {
        error = subscribe(topic);
        if (error != 0)
            return error;
    }
    _private->subscrible(topic, subscriber, token);
    return 0;
}

void Channel::unsubscribe(const Topic &topic, const Token &token)
{
    _private->unsubscrible(topic, token);
}

} // namespace net
} // namespace iot
