#include <httpclient.h>
#include <curl/curl.h>  
#include <application.h>
#include <timerfd.h>
#include <utils/utilsprint.h>
#include <string.h>

using namespace std;

namespace iot {
namespace net {
namespace http {

class SocketWatcher;

class HttpRequest::Private
{
public:
    typedef std::function<void(HttpRequest*) > Callback;
public:
    string _url;
    string _method;
    Headers _reqHeaders;
    Payload _reqBody;
    Headers _respHeaders;
    Payload _respBody;
    int _resultCode;
    Callback _completed;
};

HttpRequest::HttpRequest() :
_appData(NULL), _private(NULL)
{
}

int HttpRequest::initialize(const string& url)
{
    release();
    _private = new Private();
    _private->_url = url;
    _private->_method = "GET";
    _private->_resultCode = -1;
    return 0;
}

void HttpRequest::release()
{
    if (_private) {
        delete _private;
        _private = NULL;
    }
}

string& HttpRequest::url()
{
    return _private->_url;
}

string& HttpRequest::method()
{
    return _private->_method;
}

HttpRequest::Payload& HttpRequest::requestBody()
{
    return _private->_reqBody;
}

HttpRequest::Headers& HttpRequest::requestHeaders()
{
    return _private->_reqHeaders;
}

HttpRequest::Payload& HttpRequest::responseBody()
{
    return _private->_respBody;
}

HttpRequest::Headers& HttpRequest::responseHeaders()
{
    return _private->_respHeaders;
}

int& HttpRequest::resultCode()
{
    return _private->_resultCode;
}

HttpRequest::~HttpRequest()
{
    release();
}

class HttpClient::Private
{
public:
    Private(core::Synchronizer& synchronizer);
    virtual ~Private();
    int initRequest(HttpRequest * req);

    int appendSocket(SocketWatcher * sockp);
    int removeSocket(SocketWatcher * sockp);
    int updateSocket(SocketWatcher * sockp);

    void onTimer();
public:
    CURLM * _curlHandle;
    core::Synchronizer& _synzer;
    int running;
    TimerFD * _timer;
};

HttpClient::HttpClient() :
_private(NULL)
{
}

HttpClient::~HttpClient()
{
    release();
}

int HttpClient::initialize(core::Synchronizer& synchronizer)
{
    release();
    _private = new Private(synchronizer);
}

void HttpClient::release()
{
    if (_private) {
        delete _private;
        _private = NULL;
    }
}

int HttpClient::sendRequest(HttpRequest * req)
{
    if (_private) {
        //setup complete callback
        req->_private->_completed = std::bind(&HttpClient::_onRequestCompleted, this, placeholders::_1);
        return _private->initRequest(req);
    }
}

void HttpClient::_onRequestCompleted(HttpRequest* req)
{
    _requestCompleted(req);
}

//Per request params, associating with each CURL easy_handle

struct ConnParam
{
    CURL * easy; //CURL easy handle
    struct curl_slist * customHeaders;
    HttpRequest * request;
    HttpClient::Private * app;
    SocketWatcher * socketWatcher;
    char error[CURL_ERROR_SIZE];

public:
    ConnParam() :
    easy(NULL), customHeaders(NULL), request(NULL), app(NULL), error{0}
    {
    }

    ~ConnParam()
    {
        if (customHeaders) {
            curl_slist_free_all(customHeaders);
            customHeaders = NULL;
        }
    }

    void prepareHeaders()
    {
        auto& headers = request->requestHeaders();
        if (!headers.empty()) {
            struct curl_slist * chunk = NULL;
            for (auto it = headers.begin();
                    it != headers.end(); ++it) {
                chunk = curl_slist_append(chunk, it->c_str());
            }
            customHeaders = chunk;
        }
    }
};

//Per curl socket params

class SocketWatcher : public core::Synchronizer::Callback
{
public:

    SocketWatcher()
    {
    }

    ~SocketWatcher()
    {
        int i = 1;
    }

    void initialize(curl_socket_t sock, CURL * easy, int act, void * app)
    {
        _sockfd = sock;
        _action = act;
        _easy = easy;
        _app = (HttpClient::Private *)app;
        ConnParam * conn = NULL;
        curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
        if (conn != NULL) {
            conn->socketWatcher = this;
        }
    }
public:
    //Override Callback's methods

    void read()
    {
        readWrite();
    }

    bool wantRead() const
    {
        return (_action & CURL_POLL_IN) != 0;
    }

    void write()
    {
        readWrite();
    }

    bool wantWrite() const
    {
        return (_action & CURL_POLL_OUT) != 0;
    }

    void timeout()
    {
        curl_multi_socket_action(_app->_curlHandle, CURL_SOCKET_TIMEOUT, 0, &_app->running);
        _checkForCompleted();
    }
public:
    //Common read/write function

    void readWrite()
    {
        CURLMcode rc = curl_multi_socket_action(_app->_curlHandle, _sockfd, _action, &_app->running);
        if (rc != CURLM_OK) {
            //TODO cleanup socket error
        }
        _checkForCompleted();
    }
private:

    void _checkForCompleted()
    {
        CURLMsg *msg;
        int msgLeft;
        CURL *easyHandle;
        CURLcode res;
        ConnParam * conn;
        SocketWatcher* toBeDeleted = NULL;
        while ((msg = curl_multi_info_read(_app->_curlHandle, &msgLeft))) {
            if (toBeDeleted) {
                delete toBeDeleted;
                toBeDeleted = NULL;
            }
            if (msg->msg == CURLMSG_DONE) {
                easyHandle = msg->easy_handle;
                res = msg->data.result;
                curl_easy_getinfo(easyHandle, CURLINFO_PRIVATE, &conn);
                //cleanup easy_handle
                curl_multi_remove_handle(_app->_curlHandle, easyHandle);
                curl_easy_cleanup(easyHandle);
                //request callback
                if (conn != NULL && conn->request != NULL) {
                    conn->request->resultCode() = res;
                    if (conn->request->priv()->_completed) {
                        //beware that conn->request could be deleted in this call
                        conn->request->priv()->_completed(conn->request);
                    }
                }
                if (conn->socketWatcher) {
                    //delayed delete to make valgrind happy
                    toBeDeleted = conn->socketWatcher;
                }
                delete conn;
            }
        }
        if (toBeDeleted) {
            delete toBeDeleted;
            toBeDeleted = NULL;
        }
    }
public:
    curl_socket_t _sockfd;
    int _action;
    // long _timeout;
    CURL * _easy;
    HttpClient::Private * _app;
};

static int _multiTimerFunc(CURLM *multi, long timeout_ms, void *userp)
{
    HttpClient::Private * app = (HttpClient::Private *)userp;
    app->_timer->setTime(timeout_ms);
    return 0;
}

// CURLMOPT_SOCKETFUNCTION

static int _socketFunc(CURL *easyHandle, curl_socket_t sock, int what, void *cbp, void *sockp)
{
    try {
        HttpClient::Private * app = (HttpClient::Private *)cbp;
        SocketWatcher * sd = (SocketWatcher *) sockp;
        DPRINT("");
        if (what == CURL_POLL_REMOVE) {
            app->removeSocket(sd);
        } else if (!sockp) {
            sd = new SocketWatcher();
            sd->initialize(sock, easyHandle, what, app);
            app->appendSocket(sd);
        } else {
            sd->initialize(sock, easyHandle, what, app);
            app->updateSocket(sd);
        }
    } catch (std::exception ex) {
        return -1;
    }

    return 0;
}

HttpClient::Private::Private(core::Synchronizer& synchronizer) :
    _synzer(synchronizer)
{
    try {
        _curlHandle = curl_multi_init();
        curl_multi_setopt(_curlHandle, CURLMOPT_SOCKETFUNCTION, _socketFunc);
        curl_multi_setopt(_curlHandle, CURLMOPT_SOCKETDATA, this);
        curl_multi_setopt(_curlHandle, CURLMOPT_TIMERFUNCTION, _multiTimerFunc);
        curl_multi_setopt(_curlHandle, CURLMOPT_TIMERDATA, this);

        _timer = new TimerFD();
        _timer->setCallback(std::bind(&HttpClient::Private::onTimer, this));
        synchronizer.append(_timer->fd(), _timer);
    } catch (std::exception ex) {

    }
}

HttpClient::Private::~Private()
{
    delete _timer;
    curl_multi_cleanup(_curlHandle);
}

// CURLOPT_WRITEFUNCTION

static size_t _respBodyWriteCB(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t realsize = size * nmemb;
    ConnParam * conn = (ConnParam*) data;
    conn->request->responseBody().resize(realsize);
    memcpy((void*) conn->request->responseBody().data(), ptr, realsize);
    return realsize;
}

// CURLOPT_READFUNCTION

static size_t _reqBodyWriteCB(void *ptr, size_t size, size_t nmemb, void *data)
{
    size_t capacity = size * nmemb;
    ConnParam * conn = (ConnParam*) data;
    auto& body = conn->request->requestBody();
    if (capacity < body.size()) {
        body.resize(capacity);
    }
    size_t sz = body.size();
    if (!body.empty()) {
        memcpy(ptr, body.data(), body.size());
        body.clear();
    }
    return sz;
}

int HttpClient::Private::initRequest(HttpRequest * req)
{
    CURLMcode rc;
    ConnParam * conn = new ConnParam();
    conn->easy = curl_easy_init();
    if (!conn->easy) {
        DPRINT("curl_easy_init failed");
        delete conn;
        return -1;
    }
    conn->app = this;
    conn->request = req;
    curl_easy_setopt(conn->easy, CURLOPT_URL, req->url().c_str());
    curl_easy_setopt(conn->easy, CURLOPT_WRITEFUNCTION, &_respBodyWriteCB);
    curl_easy_setopt(conn->easy, CURLOPT_WRITEDATA, conn);
    curl_easy_setopt(conn->easy, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(conn->easy, CURLOPT_ERRORBUFFER, conn->error);
    curl_easy_setopt(conn->easy, CURLOPT_PRIVATE, conn);
    curl_easy_setopt(conn->easy, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_TIME, 3L);
    curl_easy_setopt(conn->easy, CURLOPT_LOW_SPEED_LIMIT, 10L);
    if (req->method() == "POST") {
        curl_easy_setopt(conn->easy, CURLOPT_POST, 1L); //remember to set Content-Type header
        //        curl_easy_setopt(conn->easy, CURLOPT_CUSTOMREQUEST, "POST");
    }
    if (!req->requestBody().empty()) {
        //Need to send request body (e.g. json)
        curl_easy_setopt(conn->easy, CURLOPT_POSTFIELDS, req->requestBody().c_str());
    }
    rc = curl_multi_add_handle(_curlHandle, conn->easy);
    if (rc != CURLM_OK) {
        DPRINT("curl_multi_add_handle failed");
        delete conn;
        return rc;
    }
    conn->prepareHeaders();
    if (conn->customHeaders) {
        //Need to set custom headers
        curl_easy_setopt(conn->easy, CURLOPT_HTTPHEADER, conn->customHeaders);
    }
    curl_multi_socket_action(_curlHandle, CURL_SOCKET_TIMEOUT, 0, &running);
    return 0;
}

int HttpClient::Private::appendSocket(SocketWatcher * sockp)
{
    curl_multi_assign(sockp->_app->_curlHandle, sockp->_sockfd, sockp);
    updateSocket(sockp);
    _synzer.append(sockp->_sockfd, sockp);
    return 0;
}

int HttpClient::Private::updateSocket(SocketWatcher * sockp)
{
    _synzer.update(sockp->_sockfd);
    return 0;
}

int HttpClient::Private::removeSocket(SocketWatcher * sockp)
{
    _synzer.remove(sockp->_sockfd);
    return 0;
}

void HttpClient::Private::onTimer()
{
    DPRINT("");
    curl_multi_socket_action(_curlHandle, CURL_SOCKET_TIMEOUT, 0, &running);
}
}
}
}
