#ifndef _HTTPCLIENT_H
#define _HTTPCLIENT_H

#include <stdint.h>
#include <functional>
#include <string>
#include <map>
#include <vector>
#include <core/coresynchronize.h>

namespace iot {
namespace net {
namespace http {

class HttpClient;

class HttpRequest {
public:
	typedef std::vector<std::string> Headers;
	typedef std::string Payload;
public:
	class Private;
	static inline HttpRequest * make() {
		return new HttpRequest();
	}
	virtual ~HttpRequest();
	int initialize(const std::string& url);
	void release();
	Headers& requestHeaders();
	Headers& responseHeaders();
	std::string& url();
	std::string& method();
	Payload& requestBody();
	Payload& responseBody();
	int& resultCode();
	inline void* appData() {
		return _appData;
	}
	inline void setAppData(void* data) {
		_appData = data;
	}
    Private * priv() {
        return _private;
    }
private:
	//private constructor
	//prevent direct calls, use make() instead
	HttpRequest();
	friend class HttpClient;
private:
	void* _appData;
	Private * _private;
};

class HttpClient {
public:
    HttpClient();
    virtual ~HttpClient();
    int initialize(core::Synchronizer& synchronizer);
    void release();
public:
    int sendRequest(HttpRequest * req);
private:
	//must override
	virtual void _requestCompleted(HttpRequest * req) = 0;
	
	void _onRequestCompleted(HttpRequest * req);
public:
    class Private;
private:
    Private * _private;
};

}
}
}
#endif //_HTTPCLIENT_H
