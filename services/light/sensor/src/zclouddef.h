#ifndef _ZCLOUDDEF_H
#define _ZCLOUDDEF_H

#define DEVTYPE_POWERSWITCH 1
#define DEVTYPE_SIMPLELAMP  2
#define DEVTYPE_TEMPHUM     3
#define DEVTYPE_HUM         4
#define DEVTYPE_LIGHTSENSOR 5
#define DEVTYPE_MCTEMP      6


#define CMD_POWERSWITCH     0x0101
#define CMD_HUMIDITY        0x0102
#define CMD_LIGHTSENSOR     0x2200
#define CMD_HUMIDITY        0x3100
#define CMD_TEMPERATURE     0x3300
#define CMD_MCTEMP          0x3400

#endif //_ZCLOUDDEF_H
