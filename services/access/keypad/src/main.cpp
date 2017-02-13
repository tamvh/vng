#include <iostream>
#include "application.h"
using namespace std;

int main(int argc, char *argv[])
{
    string config("/etc/vng/services/access/keypad.json");

    if (argc >= 2)
        config = argv[1];

    iot::Appication app(config);
    return app.exec(true);
}
