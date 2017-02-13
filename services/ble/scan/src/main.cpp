#include "application.h"
#include <utils/utilsprint.h>
using namespace std;

int main(int argc, char *argv[])
{
    string config("/etc/vng/services/ble/scan.json");

    if (argc >= 2)
        config = argv[1];
    PRINTF("config: %s\r\n", config.c_str());
    iot::Appication app(config);
    return app.exec(true);
}
