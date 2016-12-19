#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "bs_snmp.h"

#define SNMP_GET_REQUEST        0xA0
#define SNMP_GET_NEXT           0xA1
#define SNMP_GET_RESPONSE       0xA2
#define SNMP_SET_REQUEST        0xA3
#define SNMP_TRAP               0xA4

//https://en.wikipedia.org/wiki/X.690#BER_encoding
#define BER_BOOL        0x01
#define BER_INT         0x02
#define BER_OCTSTR      0x04
#define BER_NULL        0x05
#define BER_OBJID       0x06
#define BER_ENUM        0x0A
#define BER_SEQ         0x30
#define BER_SETOF       0x31
#define BER_IPADDR      0x40
#define BER_COUNTER     0x41
#define BER_GAUGE       0x42
#define BER_TIMETICKS   0x43
#define BER_OPAQUE      0x44


typedef struct snmp_head
{
    unsigned char pkt_flag;
    unsigned char pkt_len;
    unsigned char version_tag;
    unsigned char version_len;
    unsigned char version_value;
    unsigned char community_tag;
    unsigned char community_len;
    unsigned char community_value[6];  /*variable field*/
} snmp_head_t;

    
typedef struct snmp_pdu
{
    unsigned char pdu_type;
    unsigned char pdu_len;
    unsigned char requestid_tag;
    unsigned char requestid_len;
    unsigned char requestid_value[4];
    unsigned char error_status_tag;
    unsigned char error_status_len;
    unsigned char error_status_value;
    unsigned char error_index_tag;
    unsigned char error_index_len;
    unsigned char error_index_value;
    unsigned char item_tag1;
    unsigned char item_len1;    
    unsigned char item_tag2;
    unsigned char item_len2;
    unsigned char oid_tag;
    unsigned char oid_len;
    unsigned char oid_value[10];           /*variable field*/
}snmp_pdu_t;


typedef struct snmp_tlv
{
    uint8_t tag;
    uint8_t len;
    uint8_t value[0];
}snmp_tlv_t;

static char cpu_rate_oid[16] = {0x2b, 0x06, 0x01, 0x04, 0x01, 0x8f, 0x65, 0x0b, 0x0b, 0x00}; 
static char total_memory_oid[16] = {0x2b, 0x06, 0x01, 0x04, 0x01, 0x8f, 0x65, 0x04, 0x05, 0x00}; 
static char free_memory_oid[16] = {0x2b, 0x06, 0x01, 0x04, 0x01, 0x8f, 0x65, 0x04, 0x06, 0x00}; 
static char buffer_memory_oid[16] = {0x2b, 0x06, 0x01, 0x04, 0x01, 0x8f, 0x65, 0x04, 0x0e, 0x00}; 
static char cache_memory_oid[16] = {0x2b, 0x06, 0x01, 0x04, 0x01, 0x8f, 0x65, 0x04, 0x0f, 0x00};

int 
disp_bs_snmp(unsigned char *snmp_req, unsigned char *snmp_rsp, vbs_instance_t * vbs_inst)
{
    snmp_head_t *snmp_req_head = (snmp_head_t *)snmp_req;
    snmp_head_t *snmp_rsp_head = (snmp_head_t *)snmp_rsp;

    snmp_pdu_t *snmp_req_pdu = (snmp_pdu_t *) (snmp_req + sizeof(snmp_head_t));
    snmp_pdu_t *snmp_rsp_pdu = (snmp_pdu_t *) (snmp_rsp + sizeof(snmp_head_t));

    snmp_tlv_t *snmp_rsp_tlv = (snmp_tlv_t *) (snmp_rsp + sizeof(snmp_head_t) + sizeof(snmp_pdu_t));

    unsigned char *req_oid = snmp_req_pdu->oid_value;
    int len = sizeof(snmp_head_t) + sizeof(snmp_pdu_t) + sizeof(snmp_tlv_t);
    uint32_t value;

    if (snmp_req_pdu->pdu_type != SNMP_GET_REQUEST) {
        printf("pdu_type = %02x \n", snmp_req_pdu->pdu_type);
        return -1;
    }

    if (memcmp(req_oid, cpu_rate_oid, 10) == 0) {
        snmp_rsp_tlv->tag = BER_INT;
        snmp_rsp_tlv->len = 1;
        snmp_rsp_tlv->value[0] = vbs_inst->cpu_rate; 
    } else if (memcmp(req_oid, total_memory_oid, 10) == 0) {
        snmp_rsp_tlv->tag = BER_INT;
        snmp_rsp_tlv->len = 4;
        value = htonl(vbs_inst->mem_total);
        memcpy(snmp_rsp_tlv->value, (const void *)&value, 4);
    } else if (memcmp(req_oid, free_memory_oid, 10) == 0) {
        snmp_rsp_tlv->tag = BER_INT;
        snmp_rsp_tlv->len = 4;
        value = htonl(vbs_inst->mem_free);
        memcpy(snmp_rsp_tlv->value, (const void *)&value, 4);
        
    } else if (memcmp(req_oid, buffer_memory_oid, 10) == 0) {
        snmp_rsp_tlv->tag = BER_INT;
        snmp_rsp_tlv->len = 4;
        value = htonl(vbs_inst->mem_buffer);
        memcpy(snmp_rsp_tlv->value, (const void *)&value, 4);

    } else if (memcmp(req_oid, cache_memory_oid, 10) == 0) {
        snmp_rsp_tlv->tag = BER_INT;
        snmp_rsp_tlv->len = 4;
        value = htonl(vbs_inst->mem_cache);
        memcpy(snmp_rsp_tlv->value, (const void *)&value, 4);

    } else {
        printf("other oid \n");
        printf("req_oid: %02x %02x\n", req_oid[0], req_oid[1]); 
        return -1;
    }

    memcpy(snmp_rsp_head, snmp_req_head, sizeof(snmp_head_t));
    memcpy(snmp_rsp_pdu, snmp_req_pdu, sizeof(snmp_pdu_t));

    snmp_rsp_pdu->item_len1 += snmp_rsp_tlv->len;  
    snmp_rsp_pdu->item_len2 += snmp_rsp_tlv->len;  

    snmp_rsp_pdu->pdu_type = SNMP_GET_RESPONSE;
    snmp_rsp_pdu->pdu_len = sizeof(snmp_pdu_t) + snmp_rsp_tlv->len;
    snmp_rsp_head->pkt_len = len + snmp_rsp_tlv->len - 2;
    return len + snmp_rsp_tlv->len;
}
