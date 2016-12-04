#include "core/dev_event_timer.h"
#include "core/dev_event.h"
#include "bs_main.h"
#include "bs_utils.h"
#include "bs_vserver.h"
#include <getopt.h>

dev_event_loop_t *BsLoop = NULL;
dev_event_t *BsTimer = NULL;
dev_event_t *BSVserver = NULL;


int 
ev_loop_cb(void *data, uint32_t events)
{
    dev_event_t *ev = (dev_event_t *)data;
    //printf("events =%d\n\n", events);
    ev->handler(ev->data);
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

    struct bs_config * bsc = calloc(1, sizeof(struct bs_config));
    snprintf(bsc->ifn, sizeof(bsc->ifn), "%s", ifn);
    snprintf(bsc->ip_list_path, sizeof(bsc->ip_list_path), "%s", ip_list);

    BSVserver = bs_vserver_creat(bsc);
    if (BSVserver == NULL) {
        fprintf(stderr, "%s\n", "creat vserver fail");
        exit(-1);
    }

    dev_event_loop_add(BsLoop, BsTimer);
    dev_event_loop_add(BsLoop, BSVserver);
    dev_event_loop_run(BsLoop);
    return 0;
}

