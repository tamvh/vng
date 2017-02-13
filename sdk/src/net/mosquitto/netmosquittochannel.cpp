#include <net/mosquitto/netmosquittochannel.h>
#include <net/mosquitto/netmosquittochannel.h>
#include <net/netaddress.h>
#include <core/coreapplication.h>
#include <core/corecommand.h>
#include <core/corelock.h>
#include <core/coresynchronize.h>
#include <core/corevariant.h>
#include <utils/utilsprint.h>
#include <mosquitto.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define QOS 0
namespace iot {
namespace net {
namespace mosquitto {

class Channel::Private: public core::Synchronizer::Callback
{
public:
    class Receive: public core::Command
    {
    public:
        typedef std::vector<uint8_t> Payload;
        Receive(const net::Channel::Topic &topic,
                const uint8_t *payload,
                const net::Channel::Size &size,
                const net::Channel::Receiver &receiver):
            _topic(topic),
            _payload(size),
            _receiver(receiver) {

            memcpy(_payload.data(), payload, size);

        }

        virtual ~Receive() {

        }

        virtual void execute() {
            _receiver.call(_topic, _payload);
            delete this;
        }
    private:
        net::Channel::Topic _topic;
        Payload _payload;
        const net::Channel::Receiver &_receiver;
    };

    typedef std::vector<char> Packet;
    typedef std::map<net::Channel::Token, Packet *> Packets;
    Private(Channel &channel,
            core::Synchronizer &synchronizer,
            const net::Channel::Receiver &receiver):
        _channel(channel),
        _synchronizer(synchronizer),
        _receiver(receiver),
        _mosquitto(NULL) {

    }

    virtual void read() {
        int error = mosquitto_loop(_mosquitto, 0, 1000);
        if (error != 0)
            mosquitto_reconnect(_mosquitto);
    }

    virtual bool wantRead() const {
        return true;
    }
    virtual void write() {
        int error = mosquitto_loop(_mosquitto, 0, 1000);
        if (error != 0)
            mosquitto_reconnect(_mosquitto);

        _synchronizer.update(mosquitto_socket(_mosquitto));
    }

    virtual bool wantWrite() const {
        return ::mosquitto_want_write(_mosquitto);
    }

    virtual void timeout() {
        int error = mosquitto_loop(_mosquitto, 0, 1000);
        if (error != 0)
            mosquitto_reconnect(_mosquitto);
    }

    int intialize(const net::Address &address, const core::Variant &settings) {

        const core::Variant *id = settings.value("id");
        if (id == NULL) {
            PRINTF("*mqtt channel* 'id' value not found\r\n");
            return -1;
        }
        if (id->type() != core::Variant::TypeString) {
            PRINTF("*mqtt channel* 'id' isn't a string\r\n");
            return -1;
        }
        _id = id->toString();

        mosquitto_lib_init();
        _mosquitto = mosquitto_new(_id.c_str(), true, this);
        if (_mosquitto == NULL) {
            mosquitto_lib_cleanup();
            return -1;
        }
        const Address::Host &host = address.host();
        const Address::Port &port = address.port();

        PRINTF("*mosquitto channel* connnecting...");
        int error = ::mosquitto_connect(_mosquitto,
                                        host.c_str(),
                                        atoi(port.c_str()),
                                        20);
        if (error != 0) {
            mosquitto_destroy(_mosquitto);
            _mosquitto = NULL;
            mosquitto_lib_cleanup();
            PRINTF("failed with error: %d\r\n", errno);
            return errno;
        }
        PRINTF("successed\r\n");
        PRINTF("*mosquitto channel* synchronize appending...");
        error = _synchronizer.append(mosquitto_socket(_mosquitto), this);
        if (error != 0) {
            ::mosquitto_disconnect(_mosquitto);
            ::mosquitto_destroy(_mosquitto);
            _mosquitto = NULL;
            mosquitto_lib_cleanup();
            PRINTF("failed with error: %d\r\n", error);
            return error;
        }
        PRINTF("successed\r\n");
        mosquitto_connect_callback_set(_mosquitto, mosquitto_connect);
        mosquitto_disconnect_callback_set(_mosquitto, mosquitto_disconnect);
        mosquitto_publish_callback_set(_mosquitto, mosquitto_publish);
        mosquitto_subscribe_callback_set(_mosquitto, mosquitto_subscribe);
        mosquitto_message_callback_set(_mosquitto, mosquitto_message);
        mosquitto_unsubscribe_callback_set(_mosquitto, mosquitto_unsubscribe);
        mosquitto_log_callback_set(_mosquitto, mosquitto_log);
        return 0;
    }

    int subscribe(const net::Channel::Topic &topic) {
        PRINTF("*mosquitto channel* subscribling..");
        int id;
        int error = ::mosquitto_subscribe(_mosquitto, &id, topic.c_str(), QOS);
        if (error != 0) {
            PRINTF("*failed with error: %d\r\n", error);
            return error;
        }
        PRINTF("successed\r\n");
        return 0;
    }

    int publish(const net::Channel::Topic &topic,
                const net::Channel::Payload *payload,
                const net::Channel::Size& size,
                net::Channel::Token &token) {
        int id;
        return ::mosquitto_publish(_mosquitto,
                                   &id,
                                   topic.c_str(),
                                   size,
                                   payload,
                                   QOS,
                                   token);
    }

    int unsubscribe(const net::Channel::Topic &topic) {
        PRINTF("*mosquitto channel* unsubscribing topic: %s...", topic.c_str());
        int id;
        int error = ::mosquitto_unsubscribe(_mosquitto, &id, topic.c_str());
        if (error != 0) {
            PRINTF("*failed with error: %d\r\n", error);
            return error;
        }
        PRINTF("successed\r\n");
        return 0;
    }

    void release() {
        ::mosquitto_disconnect(_mosquitto);
        ::mosquitto_destroy(_mosquitto);
        _mosquitto = NULL;
        mosquitto_lib_cleanup();
        Packets::iterator end = _packets.end();
        Packets::iterator iterator = _packets.begin();
        for(; iterator != end; iterator ++)
            delete iterator->second;
        _packets.clear();
    }
private:
    static void mosquitto_connect(struct mosquitto *mosquitto,
                                  void *user,
                                  int error) {
        Private *channel = (Private *) user;
        channel->connected(error);
    }

    static void mosquitto_disconnect(struct mosquitto */*mosquitto*/,
                                     void *user,
                                     int error) {
        Private *channel = (Private *) user;
        channel->disconnected(error);
    }

    static void mosquitto_publish(struct mosquitto */*mosquitto*/,
                                  void *user,
                                  int id) {
        Private *channel = (Private *) user;
        channel->published(id);
    }

    static void mosquitto_message(struct mosquitto */*mosquitto*/,
                                  void *user,
                                  const struct mosquitto_message *message) {

        Private *channel = (Private *) user;
        channel->messaged(message);
    }

    static void mosquitto_subscribe(struct mosquitto */*mosquitto*/,
                                    void *user,
                                    int id,
                                    int count,
                                    const int *granteds) {
        Private *channel = (Private *) user;
        channel->subscribed(id, count, granteds);
    }

    static void mosquitto_unsubscribe(struct mosquitto */*mosquitto*/,
                                      void *user,
                                      int id) {
        Private *channel = (Private *) user;
        channel->unsubscribed(id);
    }

    static void mosquitto_log(struct mosquitto *,
                              void *user,
                              int level,
                              const char *message) {
        Private *channel = (Private *) user;
        channel->log(level, message);
    }

private:
    void connected(int error) {

    }

    void disconnected(int error) {

    }

    void published(int id) {

    }


    void messaged(const struct mosquitto_message *message) {
        Receive *receive = new Receive(message->topic,
                                       (const uint8_t *) message->payload,
                                       (net::Channel::Size) message->payloadlen,
                                       _receiver);

        if (core::Application::instance()->schedule(receive) != 0)
            delete  receive;
    }

    void subscribed(int id, int count, const int *granteds) {

    }

    void unsubscribed(int id) {

    }

    void log(int level, const char *message) {
        PRINTF("*mosquitto log* %s\r\n", message);
    }


private:
    Channel &_channel;
    core::Synchronizer &_synchronizer;
    net::Channel::Receiver _receiver;
    Packets _packets;
    std::string _id;
    struct mosquitto *_mosquitto;
};

Channel::Channel(const Address &address, const core::Variant &settings):
    net::Channel(address, settings),
    _private(NULL)
{

}

Channel::~Channel()
{
    delete _private;
}

int Channel::intialize(core::Synchronizer &synchronizer,
                       const Address &address,
                       const core::Variant &settings,
                       const net::Channel::Receiver &receiver)
{
    if (_private != NULL)
        return -1;
    _private = new Private(*this, synchronizer, receiver);
    int error = _private->intialize(address, settings);
    if (error != 0) {
        delete _private;
        _private = NULL;
        return error;
    }

    return 0;
}

int Channel::subscribe(const net::Channel::Topic &topic)
{
    return _private->subscribe(topic);
}

int Channel::publish(const net::Channel::Topic &topic,
                     const net::Channel::Payload *payload,
                     const net::Channel::Size &size,
                     net::Channel::Token &token)
{
    return _private->publish(topic, payload, size, token);
}

int Channel::unsubscribe(const net::Channel::Topic &topic)
{
    return _private->unsubscribe(topic);
}

void Channel::release()
{
    if (_private == NULL)
        return;
    _private->release();
    delete _private;
    _private = NULL;
}

} // namespace mosquitto
} // namespace net
} // namespace iot
