#ifndef CLOUDPROCESSCOMMAND_H
#define CLOUDPROCESSCOMMAND_H

#include <string>
#include <vector>
#include <core/corecommand.h>
#include <comm/commmessage.h>
#include <net/netchannel.h>
#include <ble/bledefs.h>
#include <utils/utilsprint.h>
#include <core/corecallback.h>

using namespace iot;

class CloudProcessCommand : public iot::core::Command
{
public:
    typedef iot::core::Callback<void(const comm::Message &message, net::Channel::Token &token)> DoAdvertiseCB;

    CloudProcessCommand(std::string topic,
                        uint8_t* data,
                        int dataLen,
                        const DoAdvertiseCB& advertiseCB);
    virtual ~CloudProcessCommand();
    virtual void execute();
private:
    int  processCloudMsg(const std::string& data);
    void openDoor(const ble::Address &address);
    void turnLamp(const ble::Address &address, int brigthness);
private:
    std::string          _topic;
    std::vector<uint8_t> _payload;
    DoAdvertiseCB        _doAdvertiseCB;
};

#endif // CLOUDPROCESSCOMMAND_H
