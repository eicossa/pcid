/**
 * @file network_id.c
 * @date 16 Sep 2014
 * @brief File containing network interface detection functions for Linux.
 *
 * The only public function of this module is #getAdapterInfos(OsAdapterInfo *,
 *		size_t *), other functions are either static or inline.
 *
 * Responsibility of this module is to fill OsAdapterInfo structures, in a
 * predictable way (skip "lo" interfaces,
 * @TODO: place physical interfaces in front in a repeatable order: "eth", "wlan","ib"
 * and other interfaces later, first the one with a a specified mac address, then
 * the ones with only an ip.)
 */

#ifndef _GNU_SOURCE
#define _GNU_SOURCE     /* To get defns of NI_MAXSERV and NI_MAXHOST */
#endif
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/if_link.h>
#include <netpacket/packet.h>
#include <string.h>
#include <stdio.h>
#include "ntwkId.h"

/**
 *
 * @param ifnames
 * @param ifname
 * @param ifnames_max
 * @return
 */

static int ifname_position(char *ifnames, char * ifname, int ifnames_max) 
{
	int i, position;
	position = -1;
	for (i = 0; i < ifnames_max; i++) {
		if (strcmp(ifname, &ifnames[i * NI_MAXHOST]) == 0) {
			position = i;
			break;
		}
	}
	return position;
}

/**
 * @param adapterInfos
 * @param adapter_info_size
 * @return
 */
FUNCTION_RETURN getAdapterInfos(OsAdapterInfo * adapterInfos, 
                                size_t * adapter_info_size) 
{
	FUNCTION_RETURN f_return = FUNC_RET_OK;
	struct ifaddrs *ifaddr, *ifa;
	int family, n = 0, if_name_position;
	unsigned int if_num, if_max;
	//char host[NI_MAXHOST];
	char *ifnames;

	if (getifaddrs(&ifaddr) == -1) {
		perror("getifaddrs");
		return FUNC_RET_ERROR;
	}

	if (adapterInfos != NULL) {
		memset(adapterInfos, 0, (*adapter_info_size) * sizeof(OsAdapterInfo));
	}

	/* count the maximum number of interfaces */
	for (ifa = ifaddr, if_max = 0; ifa != NULL; ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}
		if_max++;
	}

	/* allocate space for names */
	ifnames = (char*) malloc(NI_MAXHOST * if_max);
	memset(ifnames, 0, NI_MAXHOST * if_max);

	/* Walk through linked list, maintaining head pointer so we
	 can free list later */
	for (ifa = ifaddr, n = 0, if_num = 0; 
	     					 ifa != NULL;
		 		ifa = ifa->ifa_next, n++) {
		if (ifa->ifa_addr == NULL) {
			continue;
		}
		if (strcmp(ifa->ifa_name, "lo")      == 0) continue;
		if (strcmp(ifa->ifa_name, "enp2s0")  == 0) continue;
		if (strcmp(ifa->ifa_name, "docker0") == 0) continue;
		
		if_name_position = ifname_position(ifnames, ifa->ifa_name, if_num);

		// interface name not seen in advance
		if (if_name_position < 0) {
			strncpy(&ifnames[if_num * NI_MAXHOST], ifa->ifa_name, NI_MAXHOST);
			if (adapterInfos != NULL && if_num < *adapter_info_size) {
				strncpy(adapterInfos[if_num].description, ifa->ifa_name, NI_MAXHOST-1);
			}
			if_name_position = if_num;
			if_num++;
			if (adapterInfos == NULL) {
				continue;
			}
		}

		family = ifa->ifa_addr->sa_family;

		/* Display interface name and family (including symbolic
		 form of the latter for the common families) */

		//printf("(Ifname|Familyname|FamilyNum) %-8s %s (%d)\n", ifa->ifa_name,
		//		(family == AF_PACKET) ? "AF_PACKET" :
		//		(family == AF_INET) ? "AF_INET" :
		//		(family == AF_INET6) ? "AF_INET6" : "???", family);

		/* For an AF_INET* interface address, display the address
		 * || family == AF_INET6*/
		if (family == AF_INET) {
			if (adapterInfos != NULL && if_name_position < *adapter_info_size) {
				struct sockaddr_in *s1 = (struct sockaddr_in*) ifa->ifa_addr;
				in_addr_t iaddr        = s1->sin_addr.s_addr;
				adapterInfos[if_name_position].ipv4_address[0] = (iaddr	& 0x000000ff);
				adapterInfos[if_name_position].ipv4_address[1] = (iaddr	& 0x0000ff00) >> 8;
				adapterInfos[if_name_position].ipv4_address[2] = (iaddr	& 0x00ff0000) >> 16;
				adapterInfos[if_name_position].ipv4_address[3] = (iaddr	& 0xff000000) >> 24;
			}
		} else if (family == AF_PACKET && ifa->ifa_data != NULL) {
			struct sockaddr_ll *s1 = (struct sockaddr_ll*) ifa->ifa_addr;
			if (adapterInfos != NULL && if_name_position < *adapter_info_size) {
				for (int i = 0; i < 6; i++) {
					adapterInfos[if_name_position].mac_address[i] =	s1->sll_addr[i];
					//printf("%02x:", s1->sll_addr[i]);
				}
			}

			if(adapterInfos != NULL) {
				printf("IPaddr - MACaddr : %3d.%3d.%3d.%3d - %02x:%02x:%02x:%02x:%02x:%02x \n",
						adapterInfos[if_name_position].ipv4_address[0],
						adapterInfos[if_name_position].ipv4_address[1],
						adapterInfos[if_name_position].ipv4_address[2],
						adapterInfos[if_name_position].ipv4_address[3],
						adapterInfos[if_name_position].mac_address[0],
						adapterInfos[if_name_position].mac_address[1],
						adapterInfos[if_name_position].mac_address[2],
						adapterInfos[if_name_position].mac_address[3],
						adapterInfos[if_name_position].mac_address[4],
						adapterInfos[if_name_position].mac_address[5]);
			}
		}
	}

	//----this is where adapter_info_size is being set----
	*adapter_info_size = if_num;
	//----this is where adapter_info_size is being set----


	if (adapterInfos == NULL) {
		f_return = FUNC_RET_OK;
	} else if (*adapter_info_size < if_num) {
		f_return = FUNC_RET_BUFFER_TOO_SMALL;
	}
	freeifaddrs(ifaddr);
	free(ifnames);
	return f_return;
}



FUNCTION_RETURN generate_ethernet_pc_id(PcIdentifier * identifiers,
		                                       unsigned int * num_identifiers, 
											   int use_mac) 
{
	printf("generate_ethernet_pc_id() : Inside man \n");
	FUNCTION_RETURN result_adapterInfos;
	unsigned int j, k;
	OsAdapterInfo *adapterInfos;
	size_t defined_adapters, adapters = 0;

	if (identifiers == NULL || *num_identifiers == 0) {
		result_adapterInfos = getAdapterInfos(NULL, &adapters);
		if (result_adapterInfos == FUNC_RET_OK || result_adapterInfos == FUNC_RET_BUFFER_TOO_SMALL) {
			*num_identifiers = adapters;
			result_adapterInfos = FUNC_RET_OK;
		}
	} else {
		defined_adapters = adapters = *num_identifiers;
		adapterInfos = (OsAdapterInfo*) malloc(adapters * sizeof(OsAdapterInfo));
		result_adapterInfos = getAdapterInfos(adapterInfos, &adapters);
		if (result_adapterInfos == FUNC_RET_BUFFER_TOO_SMALL || result_adapterInfos == FUNC_RET_OK) {
			for (j = 0; j < adapters; j++) {
				for (k = 0; k < 6; k++)
					if (use_mac) {
						identifiers[j][k] = adapterInfos[j].mac_address[k + 2];
					} else {
						//use ip
						if (k < 4) {
							identifiers[j][k] = adapterInfos[j].ipv4_address[k];
						} else {
							//padding
							identifiers[j][k] = 42;
						}
					}
			}
			result_adapterInfos = (adapters > defined_adapters ? FUNC_RET_BUFFER_TOO_SMALL : FUNC_RET_OK);
		}
		free(adapterInfos);
	}
	return result_adapterInfos;
}
