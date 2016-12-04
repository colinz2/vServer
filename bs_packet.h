#ifndef _BS_PACKET_H
#define _BS_PACKET_H
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include "bs_main.h"

#define VBS_RESPOND_ARP     1
#define VBS_RESPOND_PING    2
#define VBS_RESPOND_SNMP    4

enum proto_type
{
    proto_arp =  VBS_RESPOND_ARP,
    proto_icmp = VBS_RESPOND_PING,
    proto_snmp = VBS_RESPOND_SNMP,
};

int 
packet_arp_reply(struct ether_arp *arp, struct ether_arp *arp_ack);



int 
pack_respond(int ptype, unsigned char *rev, unsigned char *rsp, int len);

#endif