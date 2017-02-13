#include <zcloudclient.h>
#include <stdio.h>
#include <sstream>
#include <string.h>

#include <core/corevariant.h>
#include <utils/utilsprint.h>

#include <zclouddevice.h>
#include <zclouddef.h>

using namespace iot;
using namespace iot::core;
using namespace iot::net::http;

ZCloudClient::ZCloudClient()
{
}

ZCloudClient::~ZCloudClient()
{
}

int ZCloudClient::initialize(iot::core::Synchronizer &sync, const iot::core::Variant *cfg)
{
    const core::Variant *token = cfg->value("token");
    if (token == NULL) {
        DPRINT("token missing");
        return -1;
    }
    _token = token->toString();

    const core::Variant *apiKey = cfg->value("apikey");
    if(apiKey == NULL) {
        DPRINT("apiKey missing");
        return -1;
    }
    _apiKey = apiKey->toString();

    const core::Variant *apiUrl = cfg->value("apiUrl");
    if (apiUrl == NULL) {
        DPRINT("apiUrl missing");
        return -1;
    }
    _apiURL = apiUrl->toString();
    HttpClient::initialize(sync);

    //Map cmd to cloud device type
    _cmdDevMap[CMD_TEMPERATURE] = DEVTYPE_TEMPHUM;
    _cmdDevMap[CMD_HUMIDITY] = DEVTYPE_HUM;
    _cmdDevMap[CMD_LIGHTSENSOR] = DEVTYPE_LIGHTSENSOR;
    _cmdDevMap[CMD_POWERSWITCH] = DEVTYPE_POWERSWITCH;
    _cmdDevMap[CMD_MCTEMP]  = DEVTYPE_MCTEMP;
//    _cmdDevMap[CMD_SIMPLELAMP] = DEVTYPE_SIMPLELAMP;

    requestDeviceList();
}

int ZCloudClient::deviceUpdate(const std::string &address, uint16_t cmdType, float value, int battery)
{
    //Find address in known devices from server
    //    auto iter = _devices.find(address);
    //    if (iter == _devices.end()) {
    //        DPRINT("Unknown device adress %s", address.c_str());
    //        return -1;
    //    }
    try {
        HttpRequest *req = HttpRequest::make();
        req->initialize(_apiURL + "/user/device/data/push");
        req->method() = "POST";

        uint16_t cloudDevType = 0;
        auto iter = _cmdDevMap.find(cmdType);
        if (iter != _cmdDevMap.end()) {
            cloudDevType = iter->second;
        } else {
            DPRINT("Unmapped cmdType %d", cmdType);
            return 0;
        }

        if (cmdType == CMD_TEMPERATURE ||
            cmdType == CMD_HUMIDITY ||
            cmdType == CMD_LIGHTSENSOR) //float data
        {
            DPRINT("Update %s type %d with %3.3f", address.c_str(), cloudDevType, value);
            char s[1024] = {0};
            sprintf(s, "{\"address\":\"%s\","
                       " \"sensor\": %d,"
                       " \"value\": \"%3.2f\","
                       " \"battery\": %d,"
                       " \"apikey\": \"b4704f34-d8af-11e6-a1b4-fa163e98bacf\"}",
                    address.c_str(),
                    cloudDevType,
                    value,
                    battery);

            DPRINT("date %s", s);
            req->requestBody().assign(s, s + strlen(s));
            _sendCloudRequest(req);
        }

        if(cmdType == CMD_MCTEMP) {
            DPRINT("Update %s type %d with %3.3f", address.c_str(), cloudDevType, value);
        }
    } catch (std::exception ex) {
        DPRINT("Ex http: %s", ex.what());
        return 0;
    }

    return 0;
}

int ZCloudClient::requestDeviceList()
{
    HttpRequest *req = HttpRequest::make();
    req->initialize(_apiURL + "/user/device");
    return _sendCloudRequest(req, &ZCloudClient::_gotDeviceList);
}

int ZCloudClient::requestDeviceDetail(const std::string & description)
{
    HttpRequest * req = HttpRequest::make();
    req->initialize(_apiURL + "/user/device/" + description);
    return _sendCloudRequest(req, &ZCloudClient::_gotDeviceDetail);
}

void ZCloudClient::_prepareRequest(HttpRequest *req)
{
    req->requestHeaders().push_back("Accept: application/json");
    req->requestHeaders().push_back("Content-Type: application/json");
    req->requestHeaders().push_back("Expect: ");
    if (!_token.empty()) {
        req->requestHeaders().push_back("Authorization: Bearer " + _token);
    }
}

void ZCloudClient::_requestCompleted(iot::net::http::HttpRequest *req)
{
    ResponseHandler handler = NULL;
    _handlerMutex.lock();
    auto iter = _respHandlers.find(req);
    if (iter != _respHandlers.end()) {
        handler = iter->second;
        _respHandlers.erase(iter);
    }
    _handlerMutex.unlock();
    if (handler) {
        std::string content;
        //async schedule handler is possible
        req->responseBody().swap(content);
        (this->*handler)(content);
    }
    delete req;
}

int ZCloudClient::_sendCloudRequest(iot::net::http::HttpRequest *req, ResponseHandler handler)
{
    _prepareRequest(req);
    int error = sendRequest(req);
    if (!error && handler != NULL) {
        ScopedLocker locker(_handlerMutex);
        _respHandlers[req] = handler;
        return 0;
    }
    return error;
}

static int _parseDeviceData(ZCloudDevice & dev, const Variant & cfg)
{
    const Variant * v = cfg.value("address");
    if (v != NULL && (v->type() == Variant::TypeString)) {
        dev.address = v->toString();
    } else {
        DPRINT("invalid address");
    }
    v = cfg.value("type");
    if (v != NULL && (v->type() == Variant::TypeInt)) {
        dev.type = v->toInt();
    }
    v = cfg.value("value");
    if (v != NULL && (v->type() == Variant::TypeString)) {
        dev.value = v->toString();
    }
    v = cfg.value("deviceid");
    if (v != NULL && (v->type() == Variant::TypeInt)) {
        dev.id = v->toInt();
    }
    v = cfg.value("name");
    if (v != NULL && (v->type() == Variant::TypeString)) {
        dev.name = v->toString();
    }
    v = cfg.value("group");
    if (v != NULL && (v->type() == Variant::TypeString)) {
        dev.group = v->toString();
    }
    return 0;
}

void ZCloudClient::_gotDeviceList(const std::string &deviceList)
{
    //parse json content
    bool ok = false;
    Variant v = Variant::loadJsonString(deviceList, &ok);
    if (!ok) {
        DPRINT("Json parsing failed");
        return;
    }
    if (v.type() != Variant::TypeList) {
        DPRINT("Format not expected");
        return;
    }
    Variant::List devList = v.toList();
    _devices.clear();

    ZCloudDevice dev;
    int error;
    for (auto iter = devList.begin(); iter != devList.end(); ++iter) {
        error = _parseDeviceData(dev, *iter);
        if (!error) {
            _devices.insert(std::make_pair(dev.address, dev));
        } else {
            DPRINT("parse device data failed");
        }
    }
}

void ZCloudClient::_gotDeviceDetail(const std::string& devData)
{
    ZCloudDevice dev;
    bool ok;
    Variant v = Variant::loadJsonString(devData, &ok);
    if (!ok) {
        DPRINT("parse failed");
        return;
    }
    int error = _parseDeviceData(dev, v);
    if (!error) {
        auto iter = _devices.lower_bound(dev.address);
        while (iter != _devices.end()) {
            if (iter->second.id == dev.id) {
                iter->second = dev;
                break;
            }
            if (iter->second.address != dev.address) {
                break;
            }
        }
    }
}
