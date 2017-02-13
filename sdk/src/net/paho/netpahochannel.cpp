#include <net/paho/netpahochannel.h>
#include <net/netaddress.h>
#include <core/coreapplication.h>
#include <core/corecommand.h>
#include <core/corelock.h>
#include <core/coresynchronize.h>
#include <core/corevariant.h>
#include <MQTTClient.h>
#include <string.h>
#include <utils/utilsprint.h>
#include <vector>

namespace iot {
namespace net {
namespace paho {
#define QOS 0
class Channel::Private
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

    class Delivery: public core::Command
    {
    public:
        Delivery(const MQTTClient_deliveryToken &token, Private &priv):
            _token(token),
            _priate(priv) {


        }

        virtual ~Delivery() {

        }

        virtual void execute() {
            _priate.delivery(_token);
            delete this;
        }
    private:
        MQTTClient_deliveryToken _token;
        Private &_priate;
    };


    typedef std::vector<char> Packet;
    typedef std::map<net::Channel::Token, Packet *> Packets;
    Private(Channel &channel,
            core::Synchronizer &synchronizer,
            const net::Channel::Receiver &receiver):
        _channel(channel),
        _synchronizer(synchronizer),
        _receiver(receiver),
        _client(NULL) {

    }

    int intialize(const Address &address, const core::Variant &settings) {
        const core::Variant *id = settings.value("id");
        if (id == NULL) {
            PRINTF("*mqtt channel* 'id' value not found\r\n");
            return -1;
        }
        if (id->type() != core::Variant::TypeString) {
            PRINTF("*mqtt channel* 'id' isn't a string\r\n");
            return -1;
        }

        const Address::Host &host = address.host();
        const Address::Port &port = address.port();

        std::string string = port != ""? host + ":" + port: host;
        int error = MQTTClient_create(&_client,
                                      string.c_str(),
                                      id->toString().c_str(),
                                      MQTTCLIENT_PERSISTENCE_NONE,
                                      NULL);
        if (error != MQTTCLIENT_SUCCESS) {
            PRINTF("*mqtt channel* create client failed with error: %d\r\n", error);
            return error;
        }


        MQTTClient_connectOptions options = MQTTClient_connectOptions_initializer;
        options.keepAliveInterval = 20;
        options.cleansession = 1;
        MQTTClient_setCallbacks(_client,
                                this,
                                connectionLost,
                                messageArrived,
                                deliveryComplete);
        error = MQTTClient_connect(_client, &options);
        if (error != MQTTCLIENT_SUCCESS) {
            MQTTClient_destroy(&_client);
            _client = NULL;
            PRINTF("*mqtt channel* connect failed with error: %d\r\n", error);
            return error;
        }
        return 0;
    }

    int subscribe(const net::Channel::Topic &topic) {

        int error = MQTTClient_subscribe(_client, topic.c_str(), QOS);
        if (error == MQTTCLIENT_SUCCESS)
            return 0;

        PRINTF("*mqtt channel* subscrible failed with error: %d\r\n", error);
        return error;
    }

    int publish(const net::Channel::Topic &topic,
                const net::Channel::Payload *payload,
                const net::Channel::Size& size,
                net::Channel::Token &token) {

        MQTTClient_message message = MQTTClient_message_initializer;
        MQTTClient_deliveryToken deliveryToken;
        Packet *packet = new Packet(size);
        char *data = packet->data();
        memcpy(data, payload, size);
        message.payload = (void *)data;
        message.payloadlen = size;
        message.qos = QOS;
        message.retained = 0;

        int error = MQTTClient_publishMessage(_client,
                                              topic.c_str(),
                                              &message,
                                              &deliveryToken);
        if (error != MQTTCLIENT_SUCCESS) {
            PRINTF("*paho channel* publish failed with error: %d\r\n", error);
            delete packet;
            return error;
        }
        token = deliveryToken;
        _packets[token] = packet;
        return 0;
    }

    int unsubscribe(const net::Channel::Topic &topic) {
        return MQTTClient_unsubscribe(_client, topic.c_str());
    }

    void release() {
        MQTTClient_destroy(&_client);
        Packets::iterator end = _packets.end();
        Packets::iterator iterator = _packets.begin();
        for(; iterator != end; iterator ++)
            delete iterator->second;
        _packets.clear();
        _client = NULL;
    }
private:
    static void connectionLost(void *context, char *cause) {
        Private *channel = (Private *) context;
        channel->onConnectionLost(cause);
    }

    static int messageArrived(void* context,
                              char* topic,
                              int length,
                              MQTTClient_message* message) {
        Private *channel = (Private *) context;
        return channel->onMessageArrived(topic, length, message);
    }

    static void deliveryComplete(void* context,
                                 MQTTClient_deliveryToken token) {

        Private *channel = (Private *) context;
        channel->onDeliveryComplete(token);
    }
private:
    void onConnectionLost(char *cause) {
        PRINTF("*paho channel* connection lost\r\n");
        core::Application::instance()->exit(-10);
    }

    int onMessageArrived(char *topic,
                         int length,
                         MQTTClient_message* message) {

        net::Channel::Topic id(topic);
        Receive *receive = new Receive(id,
                                       (const uint8_t *) message->payload,
                                       (net::Channel::Size) message->payloadlen,
                                       _receiver);

        if (core::Application::instance()->schedule(receive) != 0)
            delete  receive;

        MQTTClient_freeMessage(&message);
        MQTTClient_free(topic);
        return 1;
    }

    void onDeliveryComplete(MQTTClient_deliveryToken token) {
        Delivery *delivery = new Delivery(token, *this);
        if (core::Application::instance()->schedule(delivery) != 0)
            delete  delivery;
    }

    void delivery(const MQTTClient_deliveryToken &token) {
        Packets::iterator found = _packets.find(token);
        if (found != _packets.end()) {
            Packet *packet = found->second;
            _packets.erase(token);
            delete packet;
        }
    }

private:
    Channel &_channel;
    core::Synchronizer &_synchronizer;
    net::Channel::Receiver _receiver;
    MQTTClient _client;
    Packets _packets;
    std::string _id;
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

} // namespace paho
} // namespace net
} // namespace iot

