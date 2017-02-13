#ifndef NETADDRESS_H
#define NETADDRESS_H
#include <sdkdefs.h>
#include <string>
#include <stdint.h>
struct addrinfo;
struct sockaddr;

namespace iot {

namespace core {
class Variant;
}

namespace net {

class AddressPrivate;
class SDKSHARED_EXPORT Address
{
public:
    typedef uint8_t Mac[6];
    typedef std::string Host;
    typedef std::string Port;
    Address();
    Address(const Address &address);
    Address(const Host &host, const Port &port);
    virtual ~Address();
    Address &operator = (const Address &address);
    std::string toString() const;
    const Host &host() const;
    const Host &port() const;
    int setup(const core::Variant &details);
    int setup(const sockaddr &addr);
private:
    AddressPrivate *_private;
};

} // namespace net
} // namespace iot

#endif // NETADDRESS_H
