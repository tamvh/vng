#ifndef APPLICATION_H
#define APPLICATION_H
#include <net/netapplication.h>
namespace iot {

class Appication : public iot::net::Application
{
public:
    Appication(const std::string &config);

    virtual ~Appication();
private:
    virtual int initialize(iot::core::Synchronizer& synchronizer,
                           const iot::core::Variant &settings,
                           iot::net::Channel &channel);

    virtual void release(iot::net::Channel &channel);
private:
    class Private;
    Private *_private;
};
} // namespace iot
#endif // APPLICATION_H
