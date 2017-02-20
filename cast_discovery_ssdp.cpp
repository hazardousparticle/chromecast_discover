#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

#include <unistd.h>

#include <cstdio>
#include <cstdlib>

#include <cstring>

#include "cast_discovery.h"

#ifdef PARSE_DEVICE_XML
#include "curl_wrapper.h"
#endif // PARSE_DEVICE_XML


#define MAX_BUF_LEN 1024

//time out per received ssdp response
//#define SOCKET_TIMEOUT 100000
#define SOCKET_TIMEOUT_S 3
#define MAX_ATTEMPTS 5

#define SSDP_PORT 1900
#define SSDP_MULTICAST "239.255.255.250"

using namespace std;


castDevices* ssdpDiscoverDevices()
{
	//SSDP socket
	int ssdp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	//address
	sockaddr_in mcast_addr, local_addr;


	if (ssdp_sock < 0)
	{
		//error making socket
		return NULL;
	}

	mcast_addr.sin_family = AF_INET;
	//add ip to the socket
	inet_pton(AF_INET, SSDP_MULTICAST, &(mcast_addr.sin_addr));

	mcast_addr.sin_port = htons(SSDP_PORT);


	local_addr.sin_family = AF_INET;
	local_addr.sin_addr.s_addr = INADDR_ANY;
	local_addr.sin_port = 0;

	unsigned int szLocalAddr = sizeof(local_addr);


	//bind for responses
	bind(ssdp_sock, (sockaddr*)&local_addr, szLocalAddr);
	getsockname(ssdp_sock, (sockaddr*)&local_addr, &szLocalAddr);


	//set the timeout, socket will die if nothing for this long
	timeval tv;
	tv.tv_sec = SOCKET_TIMEOUT_S;
	tv.tv_usec = 0;//SOCKET_TIMEOUT;
	if (setsockopt(ssdp_sock, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
	    perror("Error");
	}

	//unsigned short responsePort = ntohs(local_addr.sin_port);

	//prepare the broadcast message
	std::string ssdpSearchMsg = "M-SEARCH * HTTP/1.1\r\n";
	ssdpSearchMsg.append("HOST: ");
	ssdpSearchMsg.append(SSDP_MULTICAST);
	ssdpSearchMsg.append(":");
	ssdpSearchMsg.append("1900");
	ssdpSearchMsg.append("\r\nMAN: \"ssdp:discover\"\r\n");
	ssdpSearchMsg.append("MX: 1\r\n");
	ssdpSearchMsg.append("ST: urn:dial-multiscreen-org:service:dial:1\r\n\r\n");

    castDevices* RootDev = NULL;

    for (int i = 0 ; i < MAX_ATTEMPTS; i++)
    {
        //send the message over UDP
        if (sendto(ssdp_sock, ssdpSearchMsg.c_str(), ssdpSearchMsg.size(), 0, \
                (sockaddr *)&mcast_addr, sizeof(mcast_addr)) < 0)
        {
            //return NULL;
            //failed to send
            continue;
        }

        char RecvBuf[MAX_BUF_LEN];

        sockaddr_in device;


        int result = recvfrom(ssdp_sock, RecvBuf, sizeof RecvBuf, 0, (sockaddr *) & device, &szLocalAddr);
        // receive sockets, and put senders IP in buffer

        if (result >= MAX_BUF_LEN)
        {
            result = MAX_BUF_LEN -1;
        }

        if (result < 0)
        {
            // no response
            cout << "No response, try again." << endl;
            continue;
        }
        RecvBuf[result] = 0;

        char* cast_address = inet_ntoa(device.sin_addr);

        //cout << cast_address << " sends: " << endl << RecvBuf << endl;

        //Root
        castDevices* tmpDev = RootDev;
        RootDev = new castDevices();

        RootDev->IP_Address = new char[strlen(cast_address) + 1] {0};
        strncpy(RootDev->IP_Address, cast_address, strlen(cast_address));


        const string response = string(RecvBuf);

        int indexStart = response.find("USN: uuid:");
        int indexEnd = response.find("\r\n", indexStart);
        int lenUsn = indexEnd - indexStart;


        string tmpStr = response.substr(indexStart, lenUsn);

        RootDev->USN = new char[lenUsn + 1] {0};
        snprintf(RootDev->USN, lenUsn, tmpStr.c_str());

#ifdef PARSE_DEVICE_XML
        //read the xml sent back

        //get the url:
        string findStr = "LOCATION: ";

        indexStart = response.find(findStr);
        indexEnd = response.find("\r\n", indexStart);


        lenUsn = indexEnd - indexStart;
        tmpStr = response.substr(indexStart, lenUsn);
        tmpStr = tmpStr.substr(findStr.length(), tmpStr.length() - findStr.length());
        //url found
        cout << "Info URL: " << tmpStr << endl;

        //TODO: parse the XML to get the name.
        char* xml = HTTPRequest(tmpStr.c_str());

        if (xml == NULL)
        {
            RootDev = NULL;
            break;
        }

        free(xml);
#endif
        //move to next dev (if found)
        RootDev->nextDev = tmpDev;

    }

    close(ssdp_sock);

	return RootDev;
}

castDevices* castDevices::getHeadDevice()
{
	return headDev;
}
