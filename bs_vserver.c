#include "bs_utils.h"
#include "bs_main.h"
#include "bs_vserver.h"
#include "bs_log.h"
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
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>

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

    uint32_t A = ntohl(aa->ipaddr);
    uint32_t B = ntohl(bb->ipaddr);
    if (A > B) {
        return 1;
    } else if (A ==  B) {
        return 0;
    } else {
        return -1;
    }
}

int 
addr_binary_search(struct vbs_instance_array *a, uint32_t ipaddr) {
    int left = 0;
    int right = a->size - 1;
    while (left <= right) {
        int mid = left + ((right - left) >> 1);
        if (ntohl(a->array[mid].ipaddr) == ntohl(ipaddr)) {
            return mid;
        } else if (ntohl(a->array[mid].ipaddr) > ntohl(ipaddr)) {
            right = mid - 1;
        }  else {
            left = mid + 1;
        }
    }
    return -1;
}

int addr_search(struct vbs_instance_array *a, const char *ip)
{
    uint32_t iip = 0;
    int ret = inet_pton(AF_INET, ip, &iip);
    if(ret <= 0) {
        fprintf(stderr, "inet_pton :%s\n", strerror(errno));
        return -1;
    }
    return addr_binary_search(a, iip);
}

void addr_print(struct vbs_instance_array *a)
{
    int i;
    char ip[32] = {0};
    vbs_instance_t *vbs_inst;

    console_print("\033[2J\033[0;0H""Ip address number = %d \n", a->size);
    for (i = 0; i < a->size; i++) {
        vbs_inst = &a->array[i];
        int state = vbs_inst->stat;
        inet_ntop(AF_INET, &vbs_inst->ipaddr, ip, sizeof(ip));
        console_print("#%-4d:  %-16s | arp:(%d)%-5s ping:(%d)%-5s snmp:(%d)%-5s\n", \
                        i + 1, ip, 
                        vbs_inst->arp_count, IS_RESPOND_ARP(state)?"on":"off",
                        vbs_inst->ping_count, IS_RESPOND_PING(state)?"on":"off",
                        vbs_inst->snmp_count, IS_RESPOND_SNMP(state)?"on":"off"
                        );
    }
    console_print("\n");
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
addr_add(struct vbs_instance_array *a, vbs_instance_t *i)
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
addr_del(struct vbs_instance_array *a, vbs_instance_t *i)
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

static void 
print_hex(unsigned char *hex, int len) {
    int i;
    for (i = 0; i < len; i++) {
        if (i%32 == 0 && i) {
            printf("\n");
        }
        printf("%02hhx ", hex[i]);
 
    }
    printf("\n");
}

static void 
log_hex(unsigned char *hex, int len) 
{
    int i;
    char buf[512] = {0};
    int wlen = 0;
    for (i = 0; i < len; i++) {
        if (i%32 == 0 && i) {
            log_print("%s\n", buf);
            memset(buf, 0, sizeof(buf));
            wlen = 0;
        }
        wlen += snprintf(buf+wlen, sizeof(buf), "%02hhx ", hex[i]);
    }
    log_print("%s\n", buf);
}

void 
bs_vserver_config(unsigned char *data)
{
    printf("data:%s\n", data);
}


int 
vsever_handler(void *data)
{
    bs_vserver_t *bs_vsrv = (bs_vserver_t *)data;
    int sock_fd = bs_vsrv->fd;
    ssize_t recv_len = 0, send_len = 0;
    int protocl_type = -1;
    const int ether_header_len = sizeof(struct ether_header);

    struct sockaddr_ll from_addr;
    unsigned int addr_len = sizeof(from_addr);

    //recv_len = recvfrom(sock_fd, BufferRec, BUFFER_SIZE, 0, (void *)&from_addr, &addr_len);
    recv_len = read(sock_fd, BufferRec, BUFFER_SIZE);
    if (unlikely(recv_len < 0)) {
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
    uint16_t target_port;

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
                icmp_pack = (struct icmphdr *)ip_payload;
                if (icmp_pack->type ==  ICMP_ECHO) {
                    protocl_type = proto_icmp;
                }
            } else if (ip_pack->protocol == IPPROTO_UDP) {
                udp_pack = (struct udphdr *)ip_payload;
                target_port = ntohs(udp_pack->dest);
                if (target_port == SNMP_PORT) {
                    protocl_type = proto_snmp;
                    //printf("ntohs(udp_pack->dest) = %d \n", ntohs(udp_pack->dest));
                } else if (target_port == CONFIG_PORT) {
                    bs_vserver_config(ip_payload + sizeof(struct udphdr));
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
            vbs_instance_t * vbs_inst = &bs_vsrv->vbsa->array[idx];

            inet_ntop(AF_INET, target_ip, ip, sizeof(ip));
            log_print("target_ip = %s, stat = %d, protocl_type  = %d\n", ip, vbs_inst->stat, protocl_type);
            if (vbs_inst->stat & protocl_type) {
                log_print("recv\n");
                log_hex(BufferRec, recv_len);
                memcpy(BufferSend, BufferRec, ether_header_len);
                swap_array(ethd_s->ether_dhost, ethd_s->ether_shost, 6);
                send_len += ether_header_len;     
                if (protocl_type & proto_arp) {
                    struct ether_arp *arp = (struct ether_arp *)rsp_payload;
                    send_len += pack_respond_arp(ether_payload, rsp_payload, 0);
                    memcpy(ethd_s->ether_shost, arp->arp_sha, 6);
                    vbs_inst->arp_count++;
                } else if (protocl_type & proto_icmp) {
                    send_len += pack_respond_icmp(ether_payload, rsp_payload, 0);
                    vbs_inst->ping_count++;
                } else {
                    return 0;
                }
                log_print("send\n");
                log_hex(BufferSend, send_len);
                log_print("\n");
                
                //int ret = sendto(sock_fd, BufferSend, send_len, 0, (struct sockaddr *)&from_addr, addr_len);
                int ret = write(sock_fd, BufferSend, send_len);
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
        f = fopen(path, "w");
        if (f == NULL) {
            fprintf(stderr, "can not creat %s\n", path);
        }
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
        if (vbst.ipaddr) addr_add(a, &vbst);  
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

int
save_ifn(const char *ifn) 
{
    FILE *f = fopen("/tmp/back_server_ifn_", "w");
    if (f == NULL) {
        return -1;
    }
    fprintf(f, "%s\n", ifn);
    fclose(f);
    return 0;
}

dev_event_t * 
bs_vserver_creat(void *data)
{
    struct sockaddr_ll saddr; 
    dev_event_t* vs;
    int fd;
    struct bs_global *bsg = (struct bs_global *)data;
    bs_vserver_t *bs_vserver = calloc(1, sizeof(bs_vserver_t));
    struct ifreq *ifr = &bs_vserver->ifr;

    fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (fd < 0) {
        fprintf(stderr, "%s\n", "raw socket error");
        return NULL;
    }

    strcpy(ifr->ifr_name, bsg->ifn);
    save_ifn(bsg->ifn);
    get_if_index(fd, ifr);
    saddr.sll_family = AF_PACKET;
    saddr.sll_ifindex = ifr->ifr_ifindex;
    saddr.sll_protocol = htons(ETH_P_ALL);

    if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        close(fd);
        free(bs_vserver);
        return NULL;
    }

    if (set_if_promisc(fd, ifr, 1)   ||
        set_blocking(fd)             ||
        set_sndbuf(fd, 1024*1024*10) ||
        set_rcvbuf(fd, 1024*1024*10)) {
        fprintf(stderr, "setting error :%s\n", strerror(errno));
        close(fd);
        free(bs_vserver);
        return NULL;
    }
    //printf(" snbbuf %d \n", get_sndbuf(fd));
    
    bs_vserver->fd = fd;
    bs_vserver->vbsa = vbs_instance_array_init();
    bsg->vbs_instances = bs_vserver->vbsa;
    get_if_macaddr(fd, ifr, bs_vserver->if_mac, sizeof(bs_vserver->if_mac));
    init_ip_list(bs_vserver->vbsa, bsg->ip_list_path);

    vs = dev_event_creat(fd, EPOLLIN, vsever_handler, (void *)bs_vserver, 0);
    return vs;
}