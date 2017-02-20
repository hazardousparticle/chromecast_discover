/***
  This file is part of avahi.

  avahi is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  avahi is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General
  Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with avahi; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#include "cast_discovery.h"

#include <stdlib.h>
#include <stdio.h>
#include <cstring>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include <avahi-common/simple-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>


#define GOOGLECAST_SEARCH_STRING "_googlecast._tcp"



static castDevices* devsFound = NULL;


static AvahiSimplePoll *simple_poll = NULL;
static unsigned int devices = 0;

#define MAX_DEVICES 1
#define SEARCH_TIMEOUT 10


static void resolve_callback(
    AvahiServiceResolver *r,
    AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol,
    AvahiResolverEvent event,
    const char *name,
    const char *type,
    const char *domain,
    const char *host_name,
    const AvahiAddress *address,
    uint16_t port,
    AvahiStringList *txt,
    AvahiLookupResultFlags flags,
    void* userdata) {

    //assert(r);

    /* Called whenever a service has been resolved successfully or timed out */

    //TODO: make it call an event if it times out.

    switch (event) {
        case AVAHI_RESOLVER_FAILURE:
            fprintf(stderr, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s\n", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
            break;

        case AVAHI_RESOLVER_FOUND: {
            char a[AVAHI_ADDRESS_STR_MAX], *t;

            printf("Service '%s' of type '%s' in domain '%s':\n", name, type, domain);

            avahi_address_snprint(a, sizeof(a), address);
            t = avahi_string_list_to_string(txt);
            printf("\t%s:%u (%s)\n"
                    "\tTXT=%s\n",
                    host_name, port, a,
                    t);

            castDevices* tmpDev = devsFound;

            //TODO: make sure it is actually a CC device
            devsFound = new castDevices();

            //code to get USN from TXT field
            auto getUSN = [](AvahiStringList *txt)
			{
				char *id = NULL;

				//length of string containing name of each field (TXT)
				#define KEY_LEN 3

            	while (txt != NULL)
				{
					//key value pairs of the avahi txt field
					char *field = calloc( txt->size +1, sizeof(char));
					strncpy(field, (char*)txt->text, txt->size);

					//copy just the key, value later
					char *key = calloc(KEY_LEN, sizeof(char));
					strncpy(key, field, KEY_LEN -1 );

					if (strncmp(key, "id", KEY_LEN -1))
					{
						//if field is not the id
						printf("%s\n", field);
						txt = txt->next;
						free(field);
						free(key);
						continue;
					}
					else
					{
						//it is the id
						id = new char[strlen(field) - KEY_LEN +1]{0};

						strncpy(id, field + KEY_LEN, strlen(field) - KEY_LEN);


						free(field);
						free(key);
						break;
					}
				}

            	return (char*)id;
			};


            devsFound->USN = getUSN(txt);

            //fix to make string safer. remove excess chars
            auto strngFix = [](char* dodgyString)
			{
				int len = strlen(dodgyString);

				char *newString = new char[len + 1] {0};

				strncpy(newString, dodgyString, len);
            	return newString;
			};

            devsFound->IP_Address = strngFix(a);
            devsFound->deviceName = strngFix((char*)name);
            devsFound->nextDev = tmpDev;

            avahi_free(t);
            devices++;
        }
    }

    if (devices >= MAX_DEVICES)
    {
    	avahi_simple_poll_quit(simple_poll);
    }

    avahi_service_resolver_free(r);
}

static void browse_callback(
    AvahiServiceBrowser *b,
    AvahiIfIndex interface,
    AvahiProtocol protocol,
    AvahiBrowserEvent event,
    const char *name,
    const char *type,
    const char *domain,
    AvahiLookupResultFlags flags,
    void* userdata) {

    AvahiClient *c = userdata;
    assert(b);

    /* Called whenever a new services becomes available on the LAN or is removed from the LAN */

    if (event == AVAHI_BROWSER_NEW)
    {

    	if (!(avahi_service_resolver_new(c, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, 0, resolve_callback, c)))
    	    fprintf(stderr, "Failed to resolve service '%s': %s\n", name, avahi_strerror(avahi_client_errno(c)));
    }
}

castDevices* mdnsDiscoverDevices()
{


	AvahiClient *client = NULL;
    AvahiServiceBrowser *sb = NULL;

    /* Allocate main loop object */
    if (!(simple_poll = avahi_simple_poll_new())) {
        fprintf(stderr, "Failed to create simple poll object.\n");
        //failure
    }

    /* Allocate a new client */
    client = avahi_client_new(avahi_simple_poll_get(simple_poll), 0, (void*)NULL, NULL, (int*)NULL);

    /* Check whether creating the client object succeeded */
    if (!client) {
        //failure
    	return 0;

    }

    /* Create the service browser */
    if (!avahi_service_browser_new(client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, \
    		GOOGLECAST_SEARCH_STRING, \
    		NULL, 0, browse_callback, client)) {
    	//failure
    	return 0;
    }

    /* Run the main loop */
    avahi_simple_poll_loop(simple_poll);


    /* Cleanup things */
    if (sb)
        avahi_service_browser_free(sb);

    if (client)
        avahi_client_free(client);

    if (simple_poll)
        avahi_simple_poll_free(simple_poll);

    //TODO: fix: not everything being freed

    return devsFound;
}
