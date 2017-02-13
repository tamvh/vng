#include <net/netaddress.h>
#include <core/corevariant.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>

#ifdef USE_LIBCOAP
#include "coap/coap.h"
#endif

namespace iot {
namespace net {
class AddressPrivate
{
public:
    AddressPrivate() {

    }

    AddressPrivate(const Address::Host &host, const Address::Port &port):
        _host(host),
        _port(port) {
    }

    std::string toString() const {
        return _host + ":" + _port;
    }

    AddressPrivate *clone() const {
        return new AddressPrivate(_host, _port);
    }


    inline const Address::Host &host() const {
        return _host;
    }

    inline const Address::Port &port() const {
        return _port;
    }

    void copy(const AddressPrivate &address) {
        _host = address.host();
        _port = address.port();
    }

    int setup(const sockaddr &addr) {
        char port[16];
        char host[INET_ADDRSTRLEN];
        if (addr.sa_family == AF_INET) {
            const struct sockaddr_in &addrin = (const struct sockaddr_in &)addr;
            sprintf(port, "%d", htons(addrin.sin_port));

            inet_ntop(AF_INET,
                      &(addrin.sin_addr),
                      host,
                      INET_ADDRSTRLEN);

        } else if (addr.sa_family == AF_INET6) {
            struct sockaddr_in6 &addrin6 = (struct sockaddr_in6 &)addr;
            sprintf(port, "%d", htons(addrin6.sin6_port));
            inet_ntop(AF_INET6,
                      &(addrin6.sin6_addr),
                      host,
                      INET6_ADDRSTRLEN);
        } else {
            return -1;
        }
        _host = host;
        _port = port;
        return 0;
    }

    int setup(const core::Variant &details) {
        const core::Variant *host = details.value("host");
        if (host == NULL)
            return -1;
        if (host->type() != core::Variant::TypeString)
            return -1;
        _host = host->toString();

        const core::Variant *port = details.value("port");
        if (port == NULL)
            return 0;

        if (!port->isInteger())
            return -1;

        char string[16];
        sprintf(string, "%d", port->toInt());
        _port = string;
        return 0;
    }

private:
    Address::Host _host;
    Address::Port _port;
};

/**
 * @brief Address::Address
 */
Address::Address():
    _private(new AddressPrivate())
{

}

Address::Address(const Address &address):
    _private(address._private->clone())
{

}

Address::Address(const Host &host, const Port &port):
    _private(new AddressPrivate(host, port))
{

}

Address::~Address()
{
    delete _private;
}


Address &Address::operator = (const Address &address)
{
    _private->copy(*address._private);
    return *this;
}

std::string Address::toString() const
{
    return _private->toString();
}


const Address::Host &Address::host() const
{
    return _private->host();
}
const Address::Host &Address::port() const
{
    return _private->port();
}

int Address::setup(const core::Variant &details)
{
    return _private->setup(details);
}


int Address::setup(const sockaddr &address)
{
    return _private->setup(address);
}

} // namespace net
} // namespace iot
