#include "core/dev_event.h"
#include "bs_utils.h"
#include "bs_main.h"
#include "bs_vserver.h"
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 65535
static unsigned char BufferRec[BUFFER_SIZE];
static unsigned char BufferSend[BUFFER_SIZE];

#define Print(fmt, args...) \
        do { \
            fprintf(stderr, "DBG:%s(%d)-%s: "fmt"\n", __FILE__, __LINE__, __FUNCTION__, ##args); \
            fflush(stderr); \
        } while (0)

static int 
addr_cmp2(const void *a, const void *b)
{
    vbs_instance_t *aa = (vbs_instance_t *)a;
    vbs_instance_t *bb = (vbs_instance_t *)b;
    if (aa->ipaddr > bb->ipaddr) {
        return 1;
    } else if (aa->ipaddr ==  bb->ipaddr) {
        return 0;
    } else {
        return -1;
    }
}


static int 
addr_binary_search(struct vbs_instance_array *a, uint32_t ipaddr) {
    int left = 0;
    int right = a->size - 1;
    while (left <= right) {
        int mid = left + ((right - left) >> 1);
        if (a->array[mid].ipaddr == ipaddr) {
            return mid;
        } else if (a->array[mid].ipaddr > ipaddr) {
            right = mid - 1;
        }  else {
            left = mid + 1;
        }
    }
    return -1;
}

void addr_print(struct vbs_instance_array *a)
{
    int i;
    char ip[32] = {0};

    printf("size = %d\n", a->size);

    for (i = 0; i < a->size; i++) {
        inet_ntop(AF_INET, &a->array[i].ipaddr, ip, sizeof(ip));
        printf("ip = %s , stat = %d\n", ip, a->array[i].stat);
    }
}


/* ascending order */
static int 
addr_sort(struct vbs_instance_array *a)
{
    qsort(a->array, a->size, sizeof(vbs_instance_t), addr_cmp2);
    return 0;
}

static int 
instance_array_double(struct vbs_instance_array *a)
{
    vbs_instance_t *d = NULL;
    d = calloc(a->max << 1, sizeof(*d));
    if (NULL == d) {
        return -1;
    }
    memcpy(d, a->array, a->size * sizeof(*a->array));
    free(a->array);
    a->array = d;
    a->max = a->max << 1;
    return 0;
}

int 
instance_array_add(struct vbs_instance_array *a, vbs_instance_t *i)
{
    int index = 0;
    if (a->size >= a->max) {
        if (instance_array_double(a) < 0) {
            return -1;
        }
    }
    if ((index = addr_binary_search(a, i->ipaddr)) >= 0) {
        return index;
    }
    a->array[a->size].ipaddr = i->ipaddr;
    a->array[a->size].stat = i->stat;
    a->size++;
    addr_sort(a);
    return 0;
}

int 
uware_v4_ping_addr_del(struct vbs_instance_array *a, vbs_instance_t *i)
{
    int index = 0;
    index = addr_binary_search(a, i->ipaddr);
    if (index < 0) {
        return 0;
    }
    inet_aton("255.255.255.255", (struct in_addr *)&a->array[index].ipaddr);
    addr_sort(a);
    a->size--;
    return 0;
}


void 
print_hex(unsigned char *hex, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (i%64 == 0 && i) {
            printf("\n");
        }
        printf("%02hhx ", hex[i]);
 
    }
    printf("\n");
    fflush(stdout);
}

void 
swap_array(unsigned char *a, unsigned char *b, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        *a = *a ^ *b;
        *b = *a ^ *b;
        *a = *a ^ *b;
        a++; b++;
    }
}

int 
pack_respond(int ptype, unsigned char *rev, unsigned char *rsp, int len)
{
    struct ether_arp *arp;
    struct iphdr *ip;
    struct icmphdr *icmp;
    struct udphdr *udp;
    int rsp_len = 0;

    if (ptype == proto_arp) {
        arp = (struct ether_arp *)rsp;
        rsp_len = sizeof(struct ether_arp);
        memcpy(rsp, rev, sizeof(struct ether_arp));
        arp->arp_op = htons(ARPOP_REPLY);
        swap_array(arp->arp_sha, arp->arp_tha, 6);
        swap_array(arp->arp_spa, arp->arp_tpa, 4);
        memcpy(arp->arp_sha, "\x00\x01\x02\x03\x04\x05", 6);
    } else if (ptype == proto_icmp) {
        
    } else if (ptype == proto_snmp) {

    } else {
        return 0;
    }
    return rsp_len;
}


int 
vsever_handler(void *data)
{
    bs_vserver_t *bs_vsrv = (bs_vserver_t *)data;
    struct bs_config *conf = bs_vsrv->conf;
    int sock_fd = bs_vsrv->fd;
    ssize_t recv_len = 0, send_len = 0;
    int protocl_type = -1;
    const int ether_header_len = sizeof(struct ether_header);

    //unsigned int addr_len = sizeof(struct sockaddr);
    //struct sockaddr from_addr;
    struct sockaddr_ll addr_ll;
    unsigned int addr_len = sizeof(addr_ll);

    recv_len = recvfrom(sock_fd, BufferRec, BUFFER_SIZE, 0, (void *)&addr_ll, &addr_len);
    //recv_len = read(sock_fd, BufferRec, BUFFER_SIZE);
    if (recv_len < 0) {
        fprintf(stderr, "recv :%s\n", strerror(errno));
        exit(-1);
        return 1;
    }
    struct ether_header *ethd = (struct ether_header *)BufferRec;
    uint16_t ether_type = ntohs(ethd->ether_type);
    struct ether_arp *arp_pack;
    struct iphdr *ip_pack;
    struct icmphdr *icmp_pack;
    struct udphdr *udp_pack;
    uint32_t *target_ip = NULL;
    unsigned char *ether_payload = NULL;
    unsigned char *ip_payload = NULL;

    ether_payload = BufferRec + ether_header_len;

    switch (ether_type) {
        case ETHERTYPE_ARP:
            arp_pack = (struct ether_arp *)ether_payload;
            if (ntohs(arp_pack->arp_op) == ARPOP_REQUEST) {
                target_ip = (/*in_addr_t*/uint32_t *)arp_pack->arp_tpa;
                protocl_type = proto_arp;
            }
            break;
        case ETHERTYPE_IP:
            ip_pack = (struct iphdr *)ether_payload;
            target_ip = (uint32_t *)&ip_pack->daddr;
            ip_payload = ether_payload + ((int)(ip_pack->ihl)<<2);
            if (ip_pack->protocol == IPPROTO_ICMP) {
                protocl_type = proto_icmp;
            } else if (ip_pack->protocol == IPPROTO_UDP) {
                udp_pack = (struct udphdr *)ip_payload;
                if (ntohs(udp_pack->dest) == SNMP_PORT) {
                    protocl_type = proto_snmp;
                    //printf("ntohs(udp_pack->dest) = %d \n", ntohs(udp_pack->dest));
                }
            } else {
                protocl_type = -1;
                target_ip = NULL;
            }
            break;
        default:
            break;
    }

    if (target_ip != NULL && protocl_type > 0) {
        char ip[128] = {0};
        int idx = 0;
        uint32_t tip = *target_ip;
        struct ether_header *ethd_s = (struct ether_header *)BufferSend;
        unsigned char *rsp_payload = BufferSend + ether_header_len;

        if ((idx = addr_binary_search(bs_vsrv->vbsa, tip)) >= 0) {
            uint32_t stat = bs_vsrv->vbsa->array[idx].stat;
            inet_ntop(AF_INET, target_ip, ip, sizeof(ip));
            printf("target_ip = %s, stat = %d, protocl_type  = %d\n", ip, stat, protocl_type);
            if (stat & protocl_type) {
                print_hex(BufferRec, recv_len);
                memcpy(BufferSend, BufferRec, ether_header_len);
                send_len = pack_respond(protocl_type, ether_payload, rsp_payload, 0);
                send_len += ether_header_len;     
                swap_array(ethd_s->ether_dhost, ethd_s->ether_shost, 6);
                memcpy(ethd_s->ether_shost, bs_vsrv->if_mac, 6);
                print_hex(BufferSend, send_len);
                //writen(sock_fd, BufferSend, send_len);
                int ret = sendto(sock_fd, BufferSend, send_len, 0, (struct sockaddr *)&addr_ll, sizeof(struct sockaddr_ll));
                if (ret < 0) {
                    fprintf(stderr, "sendto :%s\n", strerror(errno));
                }
            } else {
                //printf("%s , type  = %d  off\n", ip,  protocl_type);
            }            
        }
    } 
    return 0;
}

int
init_ip_list(struct vbs_instance_array *a, char *path)
{
    FILE *f = NULL;
    char *p = NULL;
    char line[512];
    int n = 0;
    vbs_instance_t vbst;    

    if (path[0] == 0) return -1;

    f = fopen(path, "r");
    if (f == NULL) {
        fprintf(stderr, "list file can not open %s\n", path);
        return -1;
    }

    while (fgets(line, sizeof(line), f)) {
        n = 0;
        memset(&vbst, 0, sizeof(vbst));
        p = strtok(line, "  \t");
        while (p) {
            if (n > 4)break;
            if (n == 0) {
                if (!inet_pton(AF_INET, p, &vbst.ipaddr)) {
                    break;
                }
            } else {
                int xx = atoi(p);
                if (xx == 1 && n > 0) {
                    vbst.stat = vbst.stat | (1 << (n-1));
                }
            }
            p = strtok(NULL, "  \t");
            n++;
        } 
        instance_array_add(a, &vbst);  
    }

    addr_sort(a);

    addr_print(a);
    fclose(f);
    return 0;
}


struct vbs_instance_array *
vbs_instance_array_init(void)
{
    struct vbs_instance_array * vbs_array = calloc(1, sizeof(struct vbs_instance_array));
    if (vbs_array == NULL) return NULL;
    vbs_array->array = calloc(512, sizeof(vbs_instance_t));
    vbs_array->max = 512;
    vbs_array->size = 0;
    return vbs_array;
}

dev_event_t * 
bs_vserver_creat(void *data)
{
    dev_event_t* vs;
    int fd;
    int ret;

    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0) {
        fprintf(stderr, "%s\n", "raw socket error");
        return NULL;
    }

    struct bs_config *bsc = (struct bs_config *)data;

    if (set_if_promisc(bsc->ifn, 1)  ||
        bind_socket_if(fd, bsc->ifn) ||
        set_blocking(fd)          ||
/*        set_sndbuf(fd, 1024*1024*10) ||*/
        set_rcvbuf(fd, 1024*1024*10)) {
        fprintf(stderr, "setting error :%s\n", strerror(errno));
        return NULL;
    }


    //printf(" snbbuf %d \n", get_sndbuf(fd));

    bs_vserver_t *bs_vserver = calloc(1, sizeof(bs_vserver_t));
    bs_vserver->fd = fd;
    bs_vserver->conf = bsc;
    bs_vserver->vbsa = vbs_instance_array_init();
    get_if_macaddr(fd, bsc->ifn, bs_vserver->if_mac, sizeof(bs_vserver->if_mac));
    init_ip_list(bs_vserver->vbsa, bsc->ip_list_path);

    vs = dev_event_creat(fd, EPOLLIN | EPOLLOUT | EPOLLHUP | EPOLLERR, vsever_handler, (void *)bs_vserver, 0);
    return vs;
}
