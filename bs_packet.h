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

#define ETHERNET_H_SIZE        14
#define IP_H_SIZE              20
#define ICMP_H_SIZE             8
#define UDP_H_SIZE              8
#define PSDH_SIZE              12

struct pseudo_header
{
    int32_t source_ip;
    int32_t dest_ip;
    int8_t zero;
    int8_t protocol;
    int16_t seg_len;
};

int
pack_respond_arp(unsigned char *rev, unsigned char *rsp, int uplen_);
int 
pack_respond_icmp(unsigned char *rev, unsigned char *rsp, int uplen_);
int 
pack_respond_udp(unsigned char *rev, unsigned char *rsp, int datalen);

#endif
