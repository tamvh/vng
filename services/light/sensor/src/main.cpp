#include <iostream>
#include <string>
#include <application.h>

using namespace std;

int main(int argc, char** argv)
{
    string configFile("/etc/vng/services/light/sensor.json");
    if (argc >= 2)
        configFile = argv[1];
    cout << "Using config " << configFile << endl;
    
    Application app(configFile);
    int rc = app.exec(true);
	return rc;
}
