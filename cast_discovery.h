#ifndef CAST_DISCOVERY_H_
#define CAST_DISCOVERY_H_

#pragma once

class castDevices
{
private:
	static castDevices* headDev;
public:
	char* USN;//or ID
	castDevices* nextDev;
	char* IP_Address;
	char* deviceName;

	~castDevices();
	castDevices();

	castDevices* getHeadDevice();

};

castDevices* ssdpDiscoverDevices();
castDevices* mdnsDiscoverDevices();

#endif /* CAST_DISCOVERY_H_ */
