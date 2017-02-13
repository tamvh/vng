#ifndef UTILSPRINT_H
#define UTILSPRINT_H
#ifdef DEBUG_PRINT
    #include <stdio.h>
    #define PRINTF(FORMAT,args...) printf((const char *)(FORMAT),##args)

    #define __QUOTE(x) #x
    #define _QUOTE(x) __QUOTE(x)
    #define DPRINT(fmt, ...) printf("[" __FILE__ ":" _QUOTE(__LINE__) "@%-5s] " fmt "\r\n", __func__, ##__VA_ARGS__)
#else
    #define PRINTF(FORMAT,args...)
    #define DPRINT(fmt, ...) do {} while (0)
#endif

#endif // UTILSPRINT_H
