#include "core/dev_event_timer.h"
#include "core/dev_event.h"
#include "bs_main.h"
#include "bs_utils.h"
#include "bs_vserver.h"
#include <getopt.h>
#include <unistd.h>
#include <string.h>

dev_event_loop_t *BsLoop = NULL;
dev_event_t *BsTimer = NULL;
dev_event_t *BsVserver = NULL;
dev_event_t *BsCmd = NULL;

int 
cmd_input_hander(void *data)
{
    struct vbs_instance_array *intances = (struct vbs_instance_array *)data;
    char buffer[512];
    int idx, len;
    char *p;

    memset(buffer, 0, sizeof(buffer));
    len = read(STDIN_FILENO, buffer, sizeof(buffer));
    if (len <= 3) return 0;
    buffer[len - 1] = 0;
    if (strncmp(buffer, "get", 3) == 0) {
        p = strtok(buffer+3, " ");
        if (p == NULL) return 0;

        idx = addr_search(intances, p);
        if (idx < 0) {
            printf("Can not find %s\n", p);
        } else {
            int state = intances->array[idx].stat;
            printf("%-14s | arp:%-5s ping:%-5s snmp:%-5s\r", \
                        p, 
                        IS_RESPOND_ARP(state)?"open":"off",
                        IS_RESPOND_PING(state)?"open":"off",
                        IS_RESPOND_SNMP(state)?"open":"off"
                        );
        }
    }

    return 0;
}


dev_event_t * 
bs_cmd_creat(void *data)
{
    dev_event_t* vs;
    set_nonblocking(STDIN_FILENO);
    vs = dev_event_creat(STDIN_FILENO, EPOLLIN | EPOLLHUP | EPOLLERR, cmd_input_hander, (void *)data, 0);
    return vs;
}


int 
ev_loop_cb(void *data, uint32_t events)
{
    dev_event_t *ev = (dev_event_t *)data;
    //printf("events =%d\n\n", events);
    ev->handler(dev_event_get_data(ev));
    return 0;
}

int main(int argc, char *argv[])
{
    int ret = 0, opt;
    int opt_index = 0;
    char *ifn = NULL, *ip_list = NULL;
   
    struct option long_opts[] = {
        {"interface", required_argument, 0, 'i'},
        {"ip_list", required_argument, 0, 'l'},
    };

    while ((opt = getopt_long(argc, argv, "i:l:", long_opts, &opt_index)) != -1) {
        switch (opt) {
            case 'i':
                ifn = optarg;
                break;            
            case 'l':
                ip_list = optarg;
                break;
        }
    }

    if (ifn == NULL) ifn = "eth0";
    if (ip_list == NULL) ip_list = "./ip_list";

    if (if_not_exist(ifn)) {
        fprintf(stderr, "interface %s do not exit\n", ifn);
        exit(-1);
    }

    BsLoop = dev_event_loop_creat(10, ev_loop_cb);
    BsTimer = dev_event_timer_creat(20, NULL);
    if (BsTimer == NULL || BsLoop == NULL) {
        fprintf(stderr, "%s\n", "error\n");
        exit(-1);
    }

    struct bs_global * bsg = calloc(1, sizeof(struct bs_global));
    snprintf(bsg->ifn, sizeof(bsg->ifn), "%s", ifn);
    snprintf(bsg->ip_list_path, sizeof(bsg->ip_list_path), "%s", ip_list);

    BsVserver = bs_vserver_creat(bsg);
    if (BsVserver == NULL) {
        fprintf(stderr, "%s\n", "creat vserver fail");
        exit(-1);
    }

    BsCmd = bs_cmd_creat(bsg->vbs_instances);

    dev_event_loop_add(BsLoop, BsTimer);
    dev_event_loop_add(BsLoop, BsVserver);
    dev_event_loop_add(BsLoop, BsCmd);
    dev_event_loop_run(BsLoop);
    return 0;
}

