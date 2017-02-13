#include <net/netapplication.h>
#include <net/netaddress.h>
#include <net/netchannel.h>
#include <net/netchannel.h>
#include <net/paho/netpahochannel.h>
#include <net/mosquitto/netmosquittochannel.h>
#include <core/corevariant.h>
#include <utils/utilsprint.h>
#include <map>
#include <stdio.h>

namespace iot {
namespace net {
class Application::Private
{
public:
    Private():
        _channel(NULL) {

    }
    int initialize(core::Synchronizer &synchronizer,
                   const core::Variant &config) {

        const core::Variant *net = config.value("net");
        if (net == NULL) {
            PRINTF("*net application* 'net' value not found\r\n");
            return -1;
        }

        const core::Variant *channel = net->value("channel");
        if (channel == NULL) {
            PRINTF("*net application* 'net.channel' value not found\r\n");
            return -1;
        }

        const core::Variant *addr = channel->value("address");
        if (addr == NULL) {
            PRINTF("*net application* 'net.channel.address' value not found\r\n");
            return -1;
        }

        net::Address address;
        int error = address.setup(*addr);
        if (error != 0) {
            PRINTF("*net application* address setup failed with error: %d\r\n", error);
            return error;
        }


        const core::Variant *settings = channel->value("settings");
        if (settings == NULL)  {
            PRINTF("*net application* 'net.channel.settings' value not found\r\n");
            return -1;
        }

        const core::Variant *type = channel->value("type");
        if (type == NULL) {
            PRINTF("*net application* 'net.channel.settings.type' value not found\r\n");
            return -1;
        }

        if (type->type() != core::Variant::TypeString) {
            PRINTF("*net application* 'net.channel.type' isn't a string\r\n");
            return -1;
        }
        std::string string = type->toString();
        if (string == "PAHO") {
            _channel = new paho::Channel(address, *settings);
        } else if (string == "MOSQUITTO") {
            _channel = new mosquitto::Channel(address, *settings);
        } else {
            PRINTF("*net application* 'net.channel.settings.type' not supported\r\n");
            return -1;
        }

        error = _channel->intialize(synchronizer);
        if (error != 0) {
            PRINTF("*net application* channel initialize failed with error: %d\r\n", error);
            delete _channel;
            _channel = NULL;
            return error;
        }
        return 0;
    }

    inline Channel &channel() {
        return *_channel;
    }

    void release() {
        _channel->release();
        delete _channel;
        _channel = NULL;
    }

private:
    Channel *_channel;
};

Application::Application(const std::string &config):
    core::Application(config),
    _private(NULL)
{
}

Application::~Application()
{
}

int Application::initialize(core::Synchronizer& synchronizer,
                            const core::Variant &settings)
{
    _private = new Private();
    int error = _private->initialize(synchronizer, settings);
    if (error != 0) {
        delete _private;
        _private = NULL;
        return error;
    }

    error = initialize(synchronizer, settings, _private->channel());

    if (error != 0) {
        _private->release();
        delete _private;
        _private = NULL;
        return error;
    }
    return 0;
}

void Application::release()
{
    release(_private->channel());
    _private->release();
    delete _private;
    _private = NULL;
}


} // namespace net
} // namespace iot
