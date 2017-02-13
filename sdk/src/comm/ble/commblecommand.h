#include <core/corecommand.h>
#include <ble/bledefs.h>
namespace iot {
namespace comm {
namespace ble {

class SDKSHARED_EXPORT Command: public core::Command
{
public:
    Command(const iot::ble::Address &address);

    virtual ~Command();

    virtual void execute();

private:
    virtual void execute(const iot::ble::Address &address) = 0;
private:
    iot::ble::Address _address;
};

} // namespace ble
} // namespace net
} // namespace iot
