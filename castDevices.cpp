#include "cast_discovery.h"
#include <cstdio>

castDevices* castDevices::headDev;

castDevices::~castDevices()
{
	delete[] this->IP_Address;
	delete[] this->USN;
	if (this->deviceName)
        delete[] this->deviceName;
}


castDevices::castDevices()
{
	nextDev = NULL;
	IP_Address = NULL;
	USN = NULL;
	deviceName = NULL;
	headDev = this;
}
