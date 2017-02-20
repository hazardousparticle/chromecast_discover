#include "std_libraries.h"
#include <iostream>

#include "cast_discovery.h"

#define USE_SSDP

using namespace std;

int main(int argc, const char * argv[])
{
    cout << "Search for devices using MDNS" << endl;

	castDevices* devs = mdnsDiscoverDevices();
	castDevices* head = devs;

	while (devs)
	{
	    cout << "Device found!" << endl;
        cout << "    IP address: " << devs->IP_Address << endl;
        cout << "    USN ID: " << devs->USN << endl;
        cout << "    Name: " << devs->deviceName << endl;
		devs = devs->nextDev;
	}


	auto DestroyDevList = [](castDevices* devs)
	{
	    if (!devs)
        {
            return;
        }

		devs = devs->getHeadDevice();
		castDevices* tmpDev = devs;
		while (tmpDev)
		{
			castDevices* oldDev = tmpDev;
			tmpDev = tmpDev->nextDev;

			if (oldDev)
				delete oldDev;
		}

	};

	DestroyDevList(head);

	#ifdef USE_SSDP
	cout << endl;
	cout << "Search for devices using SSDP" << endl;
	devs = ssdpDiscoverDevices();

    while (devs)
    {
        cout << "Device found!" << endl;
        cout << "    IP address: " << devs->IP_Address << endl;
        cout << "    USN ID: " << devs->USN << endl;
        cout << "    Name: " << devs->deviceName << endl;
        devs = devs->nextDev;
    }

    if (!devs)
    {
        cout << "No SSDP devices found." << endl;
    }

    DestroyDevList(devs);
	#endif

	return 0;
}
