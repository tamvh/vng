#include <glog/logging.h>

#include <string>

#include "iotcloud.h"

#include <curl/curl.h>
#include <sstream>
#include "cloudmqttclient.h"

#include "json/json.hpp"
// for convenience
using json = nlohmann::json;

void function_pt(void *ptr, size_t size, size_t nmemb, void *stream) {
    printf("receive response %d\n", size);
}

bool IotCloud::sendStaffCard(const std::string& reader_id, const std::string &card_id)
{

    CURL *curl;
    CURLcode res;

    /* get a curl handle */
    curl = curl_easy_init();

    if(curl) {
        /* First set the URL that is about to receive our POST. This URL can
           just as well be a https:// URL if that is what should receive the
           data. */
        curl_easy_setopt(curl, CURLOPT_URL, _server_url.c_str());
        /* Now specify the POST data */
        std::string post_data;
        std::stringstream ss;
        ss << "cm=turn_on&";

        json dt = json::object();
        dt["gateway_id"] = _gw_id;
        dt["card_id"] = card_id;
        dt["device_id"] = reader_id;
        ss << "dt=" << dt.dump();
        post_data = ss.str();

        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        //curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)post_data.length());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data.c_str());

        std::cout << "*iotcloud* IotCloud::sendStaffCard: url = " << _server_url   << ", data = " << ss.str() << std::endl;

        /* Perform the request, res will get the return code */
        //curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, function_pt);
        res = curl_easy_perform(curl);
        /* Check for errors */
        if(res != CURLE_OK)
            //LOG(INFO) << "curl_easy_perform() failed: " << curl_easy_strerror(res);
            LOG(INFO) << "curl_easy_perform() failed: " << res;

        else {
            LOG(INFO) << "curl_easy_perform() success " << ss.str();
        }

        /* always cleanup */
        curl_easy_cleanup(curl);
        return true;
    }
    return false;
}

int IotCloud::initializeConfig(const iot::core::Variant *cloudConfig)
{
    if (cloudConfig == 0) {
        return false;
    }

    const core::Variant *apiUrl = cloudConfig->value("api_url");
    if (apiUrl == NULL) {
        printf("*IotCloud* initialize: apiUrl value missing\r\n");
        return -1;
    }

    const core::Variant *gatewayId = cloudConfig->value("getway_id");
    if (gatewayId == NULL) {
        printf("*IotCloud* initialize: gatewayId value missing\r\n");
        return -1;
    }

    const core::Variant *cloudmqtt = cloudConfig->value("mqtt");
    if (cloudmqtt == NULL) {
        printf("*IotCloud* initialize: cloud mqtt value missing\r\n");
        return -1;
    }

    const core::Variant *cloudMqttAddress = cloudmqtt->value("address");
    if (cloudMqttAddress == NULL) {
        printf("*IotCloud* initialize: cloudMqttAddress value missing\r\n");
        return -1;
    }

    const core::Variant *cloudMqttHost = cloudMqttAddress->value("host");
    if (cloudMqttHost == NULL) {
        printf("*IotCloud* initialize: cloudMqttHost value missing\r\n");
        return -1;
    }

    const core::Variant *cloudMqttPort = cloudMqttAddress->value("port");
    if (cloudMqttPort == NULL) {
        printf("*IotCloud* initialize: cloudMqttPort value missing\r\n");
        return -1;
    }

    setGatewayId(gatewayId->toString());
    setServerApiUrl(apiUrl->toString());
    setCloudMqtt(cloudMqttHost->toString(), cloudMqttPort->toInt());
    return 0;
}

void IotCloud::initialize(const CloudMQTTClient::Received& _received)
{
    curl_global_init(CURL_GLOBAL_ALL);

    mqtt_cloud = new CloudMQTTClient();
    //mqtt_cloud->initialize("104.199.155.90", 1883, std::bind(&IotCloud::processCloudMessage, this, std::placeholders::_1));
    //mqtt_cloud->initialize("127.0.0.1", 1883, std::bind(&IotCloud::processCloudMessage, this, std::placeholders::_1));
    mqtt_cloud->initialize(_gw_id, _cloud_mqtt_host, _cloud_mqtt_port, _received);

    std::string topic;
    topic += "vng-cloud/devices/";
    topic += _gw_id;
    topic += "/change_state/request";

    if (mqtt_cloud->subscribe(topic) == 0) {
        LOG(INFO) << "Subscribe success";
    }

    std::string lampTopic;
    lampTopic += "vng-cloud/devices/";
    lampTopic += _gw_id;
    lampTopic += "/switch_on_off/request";
    if (mqtt_cloud->subscribe(lampTopic) == 0) {
        LOG(INFO) << "Subscribe success";
    }
}

void IotCloud::uninitialize()
{
    LOG(INFO) << "Stop IotCloud";
    mqtt_cloud->uninitialize();
    curl_global_cleanup();
}

void IotCloud::setServerApiUrl(const std::string &serverUrl)
{
    _server_url = serverUrl;
}

void IotCloud::setGatewayId(const std::string &gatewayId)
{
    _gw_id = gatewayId;
}

void IotCloud::setCloudMqtt(const std::string &host, int port)
{
    _cloud_mqtt_host = host;
    _cloud_mqtt_port = port;
}

void IotCloud::processCloudMessage(uint8_t* data, int len)
{
    LOG(INFO) << "On processCloudMessage:" << data;
    //auto res = json::parse(data);

    //DeviceManager::getInstance().openDoor(res["dt"]["device_id"]);
}

IotCloud::IotCloud() :
    _server_url("http://campus.zing.vn/cloud/api/iot?"),
    _gw_id("gateway1"),
    _cloud_mqtt_host("tcp://104.199.155.90"),
    _cloud_mqtt_port(1883)
{
}

IotCloud::~IotCloud()
{
}
