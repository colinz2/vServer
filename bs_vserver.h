#ifndef _BS_VSERVER_
#define _BS_VSERVER_

#include <stdint.h>
#include "bs_main.h"

#define SNMP_PORT 161


#define VBS_RESPOND_ARP     1
#define VBS_RESPOND_ICMP    2
#define VBS_RESPOND_SNMP    4

#define IS_RESPOND_ARP(state) (state & VBS_RESPOND_ARP)
#define IS_RESPOND_ICMP(state) (state & VBS_RESPOND_ICMP)
#define IS_RESPOND_SNMP(state) (state & VBS_RESPOND_SNMP)


enum proto_type
{
    proto_arp =  VBS_RESPOND_ARP,
    proto_icmp = VBS_RESPOND_ICMP,
    proto_snmp = VBS_RESPOND_SNMP,
};


typedef struct vbs_instance
{
    uint32_t ipaddr;
    uint32_t stat;
} vbs_instance_t;

struct vbs_instance_array
{
    int max;
    int size;
    vbs_instance_t *array;
};

struct bs_config
{
    char ifn[32];
    char ip_list_path[256];
};

typedef struct _bs_vserver
{
    int fd;
    struct bs_config *conf;
    struct vbs_instance_array *vbsa;  //virtual back server array
    char if_mac[16];
} bs_vserver_t;


dev_event_t * bs_vserver_creat(void *data);

#endif 
