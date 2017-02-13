#ifndef _CLOUDCLIENT_H
#define _CLOUDCLIENT_H

#include <string>
#include <stdint.h>
#include <httpclient.h>
#include <core/corevariant.h>

class CloudClient : public iot::net::http::HttpClient {
public:
    CloudClient() {}
	virtual ~CloudClient() {}
	//Update 2 bytes devType & 4 bytes data to cloud
    virtual int deviceUpdate(const std::string& address, uint16_t devType, float value, int battery) = 0;

    //Cloud type specific config
    virtual int initialize(iot::core::Synchronizer& sync, const iot::core::Variant * cfg) = 0;

    virtual int requestDeviceList() = 0;
    virtual int requestDeviceDetail(const std::string&) = 0;

    virtual std::string type() = 0;
private:
    //override from HttpClient
	virtual void _requestCompleted(iot::net::http::HttpRequest* req) {
        delete req;
    }
};
#endif //
