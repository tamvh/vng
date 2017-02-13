class nethelper
{
public:
    nethelper();
    virtual ~nethelper();
public:
    int setupAddress(
        char *host,
        char *port,
        struct addrinfo **output,
        int socktype,
        int protocolFamily
    );
    void printAddressStructures(struct addrinfo *addr);
    void printAddress(struct addrinfo *addr);
};



