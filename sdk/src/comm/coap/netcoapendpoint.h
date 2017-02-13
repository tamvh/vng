#ifndef COMMNETCOAPENDPOINT_H
#define COMMNETCOAPENDPOINT_H
#include <comm.h>
#include <net/netendpoint.h>
namespace IoT {
namespace Core {
class Synchronizer;
}
namespace Comm {
namespace Net {
namespace CoAP {

class EndpointPrivate;
class COMMSHARED_EXPORT Endpoint: public Net::Endpoint
{
public:
    Endpoint(const Address &address,
             const Address &broadcast,
             const Core::Variant &settings);
    virtual ~Endpoint();
public:
    virtual int post(const Address &address,
                     const char *payload,
                     unsigned int length,
                     Core::Synchronizer &synchronizer);

    virtual void close(Core::Synchronizer &synchronizer);
private:
    virtual int open(Core::Synchronizer &synchronizer,
                     Handler &handler,
                     const Address &address,
                     const Core::Variant &settings);
private:
    EndpointPrivate *_private;
};

} // namespace CoAP
} // namespace Net
} // namespace Comm
} // namespace IoT
#endif // COMMNETCOAPENDPOINT_H
