#ifndef CLOUDCLIENT_H
#define CLOUDCLIENT_H

#include <map>
#include <mutex>
#include <string>
#include <cloudclient.h>
#include <zclouddevice.h>

class ZCloudClient : public CloudClient
{
public:
    typedef void (ZCloudClient::*ResponseHandler)(const std::string &);
    typedef std::mutex Locker;
    typedef std::lock_guard<Locker> ScopedLocker;

public:
    ZCloudClient();
    virtual ~ZCloudClient();
    //Update 2 bytes devType & 4 bytes data to cloud
    int deviceUpdate(const std::string &address, uint16_t devType, float value, int battery);
    //get all devices
    int requestDeviceList();
    //description supposed to be deviceid as string
    int requestDeviceDetail(const std::string & description);

    int initialize(iot::core::Synchronizer &sync, const iot::core::Variant *cfg);

    std::string type()
    {
        return "zcloud";
    }

private:
    //Send a request and register handler method for response content
    int _sendCloudRequest(iot::net::http::HttpRequest *req, ResponseHandler handler = NULL);
    void _prepareRequest(iot::net::http::HttpRequest *req);
    const ZCloudDevice & _findKnownDevice(const std::string & address,
            const std::string& devType);

    void _gotDeviceList(const std::string &deviceList);
    void _gotDeviceDetail(const std::string &deviceDetail);

private:
    //override
    void _requestCompleted(iot::net::http::HttpRequest *req);

private:
    std::string _apiURL;
    std::string _apiKey;
    std::string _token;
    std::map<void *, ResponseHandler> _respHandlers;
    Locker _handlerMutex;
    std::map<uint16_t, uint16_t> _cmdDevMap;

private:
    std::multimap<std::string, ZCloudDevice> _devices;
    //TODO Locker _deviceMutex;
};

#endif /* CLOUDCLIENT_H */
