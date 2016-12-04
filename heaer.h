struct iphdr
   {
 #if __BYTE_ORDER == __LITTLE_ENDIAN
     unsigned int ihl:4;
     unsigned int version:4;
 #elif __BYTE_ORDER == __BIG_ENDIAN
     unsigned int version:4;
     unsigned int ihl:4;
 #else
 # error "Please fix <bits/endian.h>"
 #endif
     u_int8_t tos;
     u_int16_t tot_len;
     u_int16_t id;
     u_int16_t frag_off;
     u_int8_t ttl;
     u_int8_t protocol;
     u_int16_t check;
     u_int32_t saddr;
     u_int32_t daddr;
   };

 struct icmphdr
 {
   u_int8_t type;        // message type
   u_int8_t code;        // type sub-code
   u_int16_t checksum;
   union
   {
     struct
     {
       u_int16_t id;
       u_int16_t sequence;
     } echo;         //echo datagram
     u_int32_t   gateway;    //gateway address 
     struct
     {
       u_int16_t __glibc_reserved;
       u_int16_t mtu;
     } frag;         // path mtu discovery 
   } un;
 };

#define ICMP_ECHOREPLY      0   /* Echo Reply           
 #define ICMP_DEST_UNREACH   3   /* Destination Unreachable  
 #define ICMP_SOURCE_QUENCH  4   /* Source Quench        
 #define ICMP_REDIRECT       5   /* Redirect (change route)  
 #define ICMP_ECHO       8   /* Echo Request         
 #define ICMP_TIME_EXCEEDED  11  /* Time Exceeded        
 #define ICMP_PARAMETERPROB  12  /* Parameter Problem        
 #define ICMP_TIMESTAMP      13  /* Timestamp Request        
 #define ICMP_TIMESTAMPREPLY 14  /* Timestamp Reply      
 #define ICMP_INFO_REQUEST   15  /* Information Request      
 #define ICMP_INFO_REPLY     16  /* Information Reply        
 #define ICMP_ADDRESS        17  /* Address Mask Request     
 #define ICMP_ADDRESSREPLY   18  /* Address Mask Reply       
 #define NR_ICMP_TYPES       18




    ip.ip_hl = 0x5;
    ip.ip_v = 0x4;
    ip.ip_tos = 0x0;
    ip.ip_len = sizeof(struct ip)+sizeof(struct icmp);
    ip.ip_id = htons(12830);
    ip.ip_off = 0x0;
    ip.ip_ttl = 64;
    ip.ip_p = IPPROTO_ICMP;
    ip.ip_sum = 0x0;
    ip.ip_src.s_addr = inet_addr("192.168.11.146");
    ip.ip_dst.s_addr = inet_addr("192.168.11.1");
    ip.ip_sum = in_cksum((unsigned short *)&ip, 20);
    memcpy(packet, &ip, sizeof(ip));

    icmp.icmp_type = ICMP_ECHO;
    icmp.icmp_code = 2;
    icmp.icmp_id = 1000;
    icmp.icmp_seq = 0;
    icmp.icmp_cksum = 0;
    icmp.icmp_cksum = in_cksum((unsigned short *)&icmp, sizeof(struct icmp));
    memcpy(packet + sizeof(struct ip), &icmp, sizeof(struct icmp));