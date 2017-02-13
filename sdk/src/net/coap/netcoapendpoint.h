#ifndef COMMNETCOAPENDPOINT_H
#define COMMNETCOAPENDPOINT_H
//#include <comm.h>
//#include <net/netendpoint.h>
#include <core/corevariant.h>
#include <core/corecommand.h>
#include <net/netaddress.h>
using namespace iot::net;
namespace IoT {
namespace Core {
class Synchronizer;
}
namespace Comm {
namespace Net {
namespace CoAP {

class EndpointPrivate;
class Endpoint
{
public:
    Endpoint(const Address &address,
             const Address &broadcast,
             const iot::core::Variant &settings);
    virtual ~Endpoint();
public:
    virtual int post(const Address &address,
                     const char *payload,
                     unsigned int length,
                     Core::Synchronizer &synchronizer);

    virtual void close(Core::Synchronizer &synchronizer);
    virtual int open(Core::Synchronizer &synchronizer,
                     Handler &handler,
                     const Address &address,
                     const iot::core::Variant &settings);
private:
    EndpointPrivate *_private;
};

} // namespace CoAP
} // namespace Net
} // namespace Comm
} // namespace IoT
#endif // COMMNETCOAPENDPOINT_H
