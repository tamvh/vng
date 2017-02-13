#ifndef UTILS_H
#define UTILS_H
struct addrinfo;
namespace iot {
namespace comm {
class Address;
}
namespace utils {
int setup(const comm::Address &address, int family, struct addrinfo **result);
} // Utils
} // VNG
#endif // UTILS_H
