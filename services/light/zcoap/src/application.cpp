#include "application.h"
#include <string.h>
#include <sstream>
#include <functional>
#include <core/corevariant.h>
#include <core/corecommand.h>
#include <net/netaddress.h>
#include <net/netsubscribe.h>
#include <utils/utilsprint.h>
#include <ble/gap/blegappacket.h>
//#include <zcoapworker.h>

using namespace iot;
using namespace std;

class Application::Private : public net::Subscriber
{
public:
    Private(net::Channel& channel);

public:
    int initialize(iot::core::Synchronizer &synchronizer,
                   const iot::core::Variant &settings);

    void release(iot::net::Channel &channel);

public:
    //parse incoming mqtt payload returning a Command to execute
    core::Command *parse(const comm::Message::Id &id,
                         const uint8_t *payload,
                         const Size &size);

public:
    std::string _topic;
};

Application::Application(const std::string &configFile) :
    iot::net::Application(configFile), _private(NULL)
{
}

int Application::initialize(iot::core::Synchronizer &synchronizer,
                            const iot::core::Variant &settings,
                            iot::net::Channel &channel)
{
    DPRINT("initialize");
    if (_private != NULL)
        return -1;

    return 0;
}

void Application::release(net::Channel &channel)
{
    if (_private) {
        delete _private;
        _private = NULL;
    }
}

Application::Private::Private(net::Channel& channel) :
    iot::net::Subscriber(channel)
{
}

int Application::Private::initialize(iot::core::Synchronizer &synchronizer,
                                     const iot::core::Variant &settings)
{
    const core::Variant *receive = settings.value("receive");
    if (receive == NULL) {
        DPRINT("receive value missing");
        return -1;
    }
    const core::Variant *sub = receive->value("sub");
    if (sub == NULL) {
        DPRINT("sub value missing");
        return -1;
    }
    _topic = sub->toString();
    DPRINT("topic: %s", _topic.c_str());
    int error = subscribe(_topic);
    if (error != 0) {
        DPRINT("subscribe failed with error %d", error);
        return error;
    }

    return error;
}

void Application::Private::release(iot::net::Channel &channel)
{
    unsubcrible(_topic);    
}

core::Command * Application::Private::parse(const comm::Message::Id & id,
                                            const uint8_t *payload,
                                            const Size &size)
{   
    return NULL;
}
