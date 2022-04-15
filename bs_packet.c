#include "bs_packet.h"
/*
struct ether_arp *arp;
struct iphdr *ip;
struct icmphdr *icmp;
struct udphdr *udp;
*/
static void 
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

unsigned short in_cksum(unsigned short *addr, size_t len)
{
    size_t nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char *) (&answer) = *(unsigned char *) w;
        sum += answer;
    }
    
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;
    return (answer);
}

int 
packet_arp_reply(struct ether_arp *arp, struct ether_arp *arp_ack)
{
    memcpy(arp_ack, arp, sizeof(struct ether_arp));
    arp_ack->arp_op = htons(ARPOP_REPLY);
    swap_array(arp_ack->arp_sha, arp_ack->arp_tha, 6);
    swap_array(arp_ack->arp_spa, arp_ack->arp_tpa, 4);
    memcpy(arp_ack->arp_sha, "\x00\x01", 2);
    memcpy(arp_ack->arp_sha + 2, arp_ack->arp_spa, 4);
    return sizeof(struct ether_arp);
}

int 
packet_ip(struct iphdr *ip, int payloadlen, uint8_t protocol, uint16_t id, uint32_t sip, uint32_t dip)
{
    int len = sizeof(struct iphdr);
    ip->ihl = 0x5;
    ip->version = 0x4;
    ip->tos = 0x00;
    ip->tot_len = htons(len + payloadlen);
    ip->id = id;
    ip->frag_off = 0x00;
    ip->ttl = 63;
    ip->protocol = protocol;
    ip->check = 0x00;
    ip->saddr = sip;
    ip->daddr = dip;
    ip->check = in_cksum((unsigned short *)ip, len);
    return (payloadlen + len);
}

int 
packet_ping_reply(struct icmphdr *icmp, int icmp_date_len)
{
    int len = sizeof(struct icmphdr) + icmp_date_len;
    icmp->type = ICMP_ECHOREPLY;
    icmp->code = 0;
    //icmp->un.echo.id = 1000;
    //icmp->un.echo.sequence = 0;
    icmp->checksum = 0;
    icmp->checksum = in_cksum((unsigned short *)icmp, len);
    return len;
}

int 
pack_respond_arp(unsigned char *rev, unsigned char *rsp, int uplen_)
{
    struct ether_arp *arp, *arp_ack; 
    arp = (struct ether_arp *)rev;
    arp_ack = (struct ether_arp *)rsp;
    return packet_arp_reply(arp, arp_ack);
}

int 
pack_respond_icmp(unsigned char *rev, unsigned char *rsp, int uplen_)
{
    struct iphdr *ip, *ip_ack;
    struct icmphdr *icmp, *icmp_ack;
    int len = 0;

    ip = (struct iphdr *)rev;
    ip_ack = (struct iphdr *)rsp;
    icmp = (struct icmphdr *) (rev + sizeof(struct iphdr));
    icmp_ack = (struct icmphdr *) (rsp + sizeof(struct iphdr));

    int icmp_date_len = ntohs(ip->tot_len) - sizeof(struct iphdr) - sizeof(struct icmphdr);
    memcpy((char *)icmp_ack, (char *)icmp, icmp_date_len + sizeof(struct icmphdr));
    len = packet_ping_reply(icmp_ack, icmp_date_len);
    return packet_ip(ip_ack, len, IPPROTO_ICMP, ip->id, ip->daddr, ip->saddr);
}

// struct udphdr
// {

//  struct
//  {
//    u_int16_t source;
//    u_int16_t dest;
//    u_int16_t len;
//    u_int16_t check;
//  };
// };
int 
pack_respond_udp(unsigned char *rev, unsigned char *rsp, int datalen)
{
    struct iphdr *ip, *ip_ack;
    struct udphdr *udp, *udp_ack;
    struct pseudo_header *udp_p_header;

    int len = sizeof(struct udphdr) + datalen;

    ip = (struct iphdr *)rev;
    udp = (struct udphdr *) (rev + sizeof(struct iphdr));
    ip_ack = (struct iphdr *)rsp;
    udp_p_header = (struct pseudo_header *) (rsp + sizeof(struct iphdr));
    udp_ack = (struct udphdr *) (rsp + sizeof(struct iphdr) + sizeof(struct pseudo_header));

    udp_ack->len = htons(datalen + sizeof(struct udphdr));
    udp_ack->source = udp->dest;   //hehe
    udp_ack->dest = udp->source;
    udp_ack->check = 0;

    udp_p_header->source_ip = ip->daddr;
    udp_p_header->dest_ip = ip->saddr;
    udp_p_header->zero = 0;
    udp_p_header->protocol = IPPROTO_UDP;
    udp_p_header->seg_len = htons(datalen + sizeof(struct udphdr));

    udp_ack->check = in_cksum((unsigned short *)udp_p_header, sizeof(struct pseudo_header) + len);

    ip_ack = (struct iphdr *) (rsp + sizeof(struct pseudo_header));
    return  packet_ip(ip_ack, len, IPPROTO_UDP, ip->id, ip->daddr, ip->saddr);
}


