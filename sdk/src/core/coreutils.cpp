#include <comm/commaddress.h>
#include <netdb.h>
#include <string.h>

namespace VNG {
namespace Core {

int setup(const Comm::Address &address, int family, struct addrinfo **result)
{
    Comm::Address::Host host;
    Comm::Address::Port port;
    int error = address.getNetwork(host, port);
    if (error != 0)
        return error;

    struct addrinfo hints;
    memset(&hints, 0x00, sizeof(struct addrinfo));
    hints.ai_family = family;
    hints.ai_socktype = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
    return getaddrinfo(host.c_str(), port.c_str(), &hints, result);
}

} // Utils
} // VNG
