#ifndef _APPLICATION_H
#define _APPLICATION_H

#include <net/netapplication.h>
#include <net/netchannel.h>
#include <core/coresynchronize.h>
#include <iostream>

class Application : public iot::net::Application
{
public:
    Application(const std::string& configFile);
public:
    class Private;
private:
    //Override; got the channel initialized from iot::net::Application
    virtual int initialize(iot::core::Synchronizer& synchronizer,
                           const iot::core::Variant &settings,
                           iot::net::Channel &channel);
    virtual void release(iot::net::Channel &channel);
private:
    Private *_private;
};

#endif
