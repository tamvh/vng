#ifndef _ZCLOUDDEVICE_H
#define _ZCLOUDDEVICE_H

#include <string>

class ZCloudDevice
{
public:
    std::string address;
    int type;
    int id;
    std::string name;
    std::string value;
    std::string group;
};
#endif //_ZCLOUDDEVICE_H