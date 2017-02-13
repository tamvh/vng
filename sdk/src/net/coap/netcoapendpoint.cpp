#include <net/coap/netcoapendpoint.h>
#include <net/netaddress.h>
#include <core/coresynchronize.h>
#include <core/corevariant.h>
#include <coap/coap.h>
#ifdef USE_LIBCOAP
#include "coap/coap.h"
#endif

#include <arpa/inet.h>
#include <netdb.h>
#include <list>
#include <string>
#include <iostream>
#include <stdio.h>
#define IoT_URI "IoT"
namespace IoT {
namespace Comm {
namespace Net {
namespace CoAP {
/**
 * @brief The EndpointPrivate class
 */
class EndpointPrivate: public Core::Synchronizer::Writer,
        public Core::Synchronizer::Reader

{
public:
    virtual void read() {
        coap_read(_context);
    }

    virtual bool reading() const {
        return true;
    }

    virtual void send() {
        coap_tick_t now;
        coap_queue_t *pdu = coap_peek_next(_context);
        coap_ticks(&now);
        while (pdu && pdu->t <= now - _context->sendqueue_basetime) {
            coap_retransmit(_context, coap_pop_next(_context));
            pdu = coap_peek_next(_context);
        }
    }

    virtual bool writing() const {
        return coap_peek_next(_context) != NULL;
    }

    int connect(const addrinfo &info,
                const std::string &uri,
                Core::Synchronizer &synchronizer) {

        if (_context != NULL)
            return -1;

        coap_address_t address;

        if (info.ai_addrlen > sizeof(address.addr))
            return -2;

        coap_address_init(&address);
        address.size = info.ai_addrlen;
        memcpy(&address.addr, info.ai_addr, info.ai_addrlen);

        _context = coap_new_context(&address);
        if (_context == 0)
            return -1;

        coap_resource_t *resource;
        resource = coap_resource_init((const unsigned char *)uri.c_str(),
                                      uri.length(),
                                      COAP_RESOURCE_FLAGS_NOTIFY_NON);

        if (resource == 0) {
            coap_free_context(_context);
            _context = NULL;
            return -1;
        }

        coap_register_handler(resource, COAP_REQUEST_POST, coapPost);
        coap_add_resource(_context, resource);
        coap_add_attr(resource,
                      (const unsigned char *)"self",
                      4,
                      (const unsigned char *)&(_self),
                      sizeof(_self),
                      0);
        int error = synchronizer.append(_context->sockfd, *this, *this);

        if (error != 0) {
            coap_free_context(_context);
            _context = NULL;
            return error;
        }
        return 0;
    }

    int send(addrinfo &info,
             const char *packet,
             unsigned int size,
             Core::Synchronizer &synchronizer) {

        coap_address_t address;
        if (info.ai_addrlen > sizeof(address.addr))
            return -2;

        coap_address_init(&address);
        address.size = info.ai_addrlen;
        memcpy(&address.addr, info.ai_addr, info.ai_addrlen);
        coap_pdu_t *pdu = coap_new_pdu();
        if (pdu == NULL)
            return -1;
        coap_add_option(pdu,
                        COAP_OPTION_URI_PATH,
                        3,
                        (const unsigned char *)IoT_URI);
        pdu->hdr->type = COAP_MESSAGE_NON;
        pdu->hdr->id = coap_new_message_id(_context);
        pdu->hdr->code = COAP_REQUEST_POST;
        if (coap_add_data(pdu, size, (const unsigned char *)packet) != 1) {
            coap_delete_pdu(pdu);
            return -1;
        }

        coap_tid_t id = coap_send(_context, _context->endpoint, &address, pdu);

        if (id == COAP_INVALID_TID) {
            coap_delete_pdu(pdu);
            return -1;
        } else if (pdu->hdr->type != COAP_MESSAGE_NON) {
            coap_delete_pdu(pdu);
            return -1;
        }

        synchronizer.update(_context->sockfd);
        return 0;
    }

    int send(const Address &address,
             const char *packet,
             unsigned int size,
             Core::Synchronizer &synchronizer) {

        struct addrinfo *result;
        const Address::Host &host = address.host();
        const Address::Port &port = address.port();
        addrinfo hints;
        memset(&hints, 0x00, sizeof(struct addrinfo));
        hints.ai_family = SOCK_DGRAM;
        hints.ai_socktype = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
        int error = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
        if ( error != 0 ) {
            printf("getaddrinfo: %s\n", gai_strerror(error));
            return error;
        }

        /* iterate through results until success */
        for (addrinfo *info = result; info != 0; info = info->ai_next)
            send(*info, packet, size, synchronizer);

        freeaddrinfo(result);
        return 0;
    }

private:
    void onGet(coap_context_t *context,
               coap_address_t *coapAddress,
               const char *data,
               size_t size) {
        if (_context != context)
            return;
        Address address;
        if (address.setup(coapAddress->addr.sa) != 0)
            return;
        _handler.onReceived(address, data, size);
    }

    void onPost(coap_context_t *context,
                coap_address_t *coapAddress,
                const char *data,
                size_t size) {

        if (_context != context)
            return;

        Address address;
        if (address.setup(coapAddress->addr.sa) != 0)
            return;
        _handler.onReceived(address, data, size);
    }

private:

    static void coapPost(coap_context_t  *context,
                         struct coap_resource_t *resource,
                         const coap_endpoint_t *endpoint,
                         coap_address_t *address,
                         coap_pdu_t *request,
                         str *token,
                         coap_pdu_t *response) {

        coap_attr_t *attribute = coap_find_attr(resource,
                                                (const unsigned char *)"self",
                                                4);
        if (attribute->value.length != sizeof(EndpointPrivate *)) {
            response->hdr->code = COAP_RESPONSE_CODE(204);
            return;
        }

        EndpointPrivate *priv;
        memcpy(&priv, attribute->value.s, sizeof(EndpointPrivate *));

        size_t size;
        unsigned char *data;
        if (coap_get_data(request, &size, &data) == 0) {
            response->hdr->code = COAP_RESPONSE_CODE(204);
            return;
        }
        priv->onPost(context, address, (const char *)data, size);
    }

public:
    EndpointPrivate(Comm::Net::Endpoint::Handler &handler):
        _handler(handler),
        _context(NULL),
        _self(this) {
    }

    int connect(const Address &address,
                const Core::Variant &settings,
                Core::Synchronizer &synchronizer) {
        const Core::Variant *uri = settings.value("uri");
        if (uri == NULL)
            return -1;
        if (uri->type() != Core::Variant::TypeString)
            return -1;

        _uri = uri->toString();

        struct addrinfo *result;
        const Address::Host &host = address.host();
        const Address::Port &port = address.port();
        addrinfo hints;
        memset(&hints, 0x00, sizeof(struct addrinfo));
        hints.ai_family = SOCK_DGRAM;
        hints.ai_socktype = AF_UNSPEC;
        hints.ai_flags = AI_PASSIVE | AI_NUMERICHOST;
        int error = getaddrinfo(host.c_str(), port.c_str(), &hints, &result);
        if ( error != 0 ) {
            printf("getaddrinfo: %s\n", gai_strerror(error));
            return error;
        }

        error = -1;
        /* iterate through results until success */
        for (addrinfo *info = result; info != 0; info = info->ai_next) {
            error = connect(*info, _uri, synchronizer);
            break;
        }
        freeaddrinfo(result);
        return error;
    }

    void close(Core::Synchronizer &synchronizer) {
        if (_context == NULL)
            return;
        synchronizer.remove(_context->sockfd);
        coap_free_context(_context);
        _context = NULL;
    }

private:

private:
    std::string _uri;
    Net::Endpoint::Handler &_handler;
    coap_context_t *_context;
    EndpointPrivate *_self;
};

/**
 * @brief Endpoint::Endpoint
 */
Endpoint::Endpoint(const Address &address,
                   const Address &broadcast,
                   const Core::Variant &settings):
    Net::CoAP::Endpoint(address, broadcast, settings),
    _private(0)
{

}

Endpoint::~Endpoint()
{

}

int Endpoint::open(Core::Synchronizer &synchronizer,
                   Handler &handler,
                   const Address &address,
                   const iot::core::Variant &settings)
{
    if (_private != 0)
        return -1;
    _private = new EndpointPrivate(handler);
    int error = _private->connect(address, settings, synchronizer);
    if (error != 0) {
        delete _private;
        _private = 0;
        return error;
    }
    return 0;
}

int Endpoint::post(const Address &address,
                   const char *payload,
                   unsigned int length,
                   Core::Synchronizer &synchronizer)
{
    if (_private == 0)
        return -1;
    return _private->send(address, payload, length, synchronizer);
}


void Endpoint::close(Core::Synchronizer &synchronizer)
{
    if (_private == 0)
        return;
    _private->close(synchronizer);
    delete _private;
    _private = 0;
}

} // namespace Net
} // namespace CoAP
} // namespace Comm
} // namespace IoT
