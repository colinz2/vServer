#ifndef _BS_VSERVER_
#define _BS_VSERVER_

#include <stdint.h>
#include "bs_main.h"
#include "core/dev_event.h"
#include "bs_packet.h"

#define SNMP_PORT 161
#define CONFIG_PORT 9999
#define LOG_PATH "/tmp/back_sever_log"

#define IS_RESPOND_ARP(state) (state & VBS_RESPOND_ARP)
#define IS_RESPOND_PING(state) (state & VBS_RESPOND_PING)
#define IS_RESPOND_SNMP(state) (state & VBS_RESPOND_SNMP)

typedef struct vbs_instance
{
    uint32_t ipaddr;
    uint32_t stat;
    uint32_t arp_count;
    uint32_t ping_count;
    uint32_t snmp_count;
    uint8_t  cpu_rate;
    uint32_t mem_total;
    uint32_t mem_free;
    uint32_t mem_cache;
    uint32_t mem_buffer;
} vbs_instance_t;

struct vbs_instance_array
{
    int max;
    int size;
    vbs_instance_t *array;
};

struct bs_global
{
    char ifn[32];
    char ip_list_path[256];
    struct vbs_instance_array *vbs_instances;
};

typedef struct _bs_vserver
{
    int fd;
    struct ifreq ifr;
    char if_mac[16];
    struct vbs_instance_array *vbsa;  //virtual back server array
    void *data;
} bs_vserver_t;

dev_event_t * bs_vserver_creat(void *data);
void bs_vserver_free(dev_event_t *ev);


void addr_print(struct vbs_instance_array *a);
int addr_search(struct vbs_instance_array *a, const char *ip);
int addr_add(struct vbs_instance_array *a, vbs_instance_t *i);
void addr_print(struct vbs_instance_array *a);
#endif 
