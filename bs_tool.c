#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>

static uint16_t portNum = 9999;
static char Ifn[16] = {0};

static int 
nonblocking(int fd) 
{
    int flags;
    if (-1 == (flags = fcntl(fd, F_GETFL, 0))) {
        perror("setnonblocking error");
        return -1;
    }
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int 
io_buff_len(int fd)
{
    int len = 0;
    ioctl(fd, FIONREAD, &len);
    return len;
}

static int 
io_poll(int fd, int timeout_ms) 
{
    struct pollfd fds[1];
    int ret;
    fds[0].fd = fd;
    fds[0].events = POLLIN | POLLPRI;
    if ((ret = poll(fds, 1, timeout_ms)) < 0)
        return -1;
    if (ret)
        return 1;
    // time out 
    return 0;
}

/*static inline void
api_msg_head(dev_api_msg_head_t *msg, uint8_t type, uint16_t len)
{
    msg->ver = 0x01;
    msg->cmd = type;
    if (len) msg->data_len = htons(len);
}
*/

int 
udp_client_creat_con(const char *ifn, unsigned short server_port)
{
    int ret;
    int sockfd;
    struct sockaddr_in servaddr;
    struct sockaddr_in *addr;
    struct ifreq ifr;
    socklen_t servlen = sizeof(struct sockaddr_in);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        return sockfd;
    }

    /*memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, ifn);

    if (ioctl(sockfd, SIOCGIFADDR, &ifr) < 0 ) {
        close(sockfd);
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    addr = (struct sockaddr_in *)(&ifr.ifr_addr);
    char ip[32] = {0};
    inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip)); 
    printf("ip = %s\n", ip);
    servaddr.sin_addr.s_addr =  addr->sin_addr.s_addr;
    
    ret = connect(sockfd, (struct sockaddr *)&servaddr, servlen);
    if (ret < 0) {
        close(sockfd);
        return ret;
    }*/
    
    return sockfd;
}
/*
static int 
api_cmd_sent(int slotid, uint8_t cmd, int *fd)
{
    static char g_buf[1024] = {0};
    *fd = udp_client_creat_con(slotid, portNum);
    if (*fd < 0) {
        return -1;
    }
    dev_api_msg_head_t *msg = (dev_api_msg_head_t *)g_buf;
    api_msg_head(msg, cmd, 0);
    return write(*fd, g_buf, sizeof(dev_api_msg_head_t));
}

static int 
api_cmd_receive(int fd, dev_api_msg_t *msg)
{
    int ret = io_poll(fd, 1000);
    int res = 0;
    int len = 0;
    if (ret > 0) {
        len = io_buff_len(fd);
        res = read(fd, (char *)msg, sizeof(dev_api_msg_t));
        if (res < 0) { return -1; }
    } else if (ret == 0) {
        fprintf(stderr, "%s\n", "time out");
        return -1;
    } else {
        fprintf(stderr, "%s\n", "poll error");
        return -1;
    }

    if (res > 0) {
        if (msg->header.code != 0x30) {
            fprintf(stderr, "error , %02hhx, %s\n", msg->header.code, msg->error_msg);
            return -1;
        }
    } 
    return 0;
}

static int 
api_set_master(int slotid) 
{
    int fd, ret;
    dev_api_msg_t msg;

    ret = api_cmd_sent(slotid, 1, &fd);
    if (ret < 0) {
        close(fd);
        return -1;
    }
    ret = api_cmd_receive(fd, &msg);
    if (ret < 0) {
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

*/
int
read_ifn(char *ifn) 
{
    FILE *f = fopen("/tmp/back_server_ifn_", "r");
    if (f == NULL) {
        return -1;
    }
    fscanf(f, "%s", ifn);
    fclose(f);
    return 0;
}

int main(int argc, char *argv[])
{
    int opt_index = 0, opt = 0;
    int loop = 0;
    int slotid;

    /* optional_argument */
    struct option long_opts[] = {
        {"help", no_argument, 0, 'h'},
        {"reboot", required_argument, 0, 2},
    };

    read_ifn(Ifn);
    printf("%s\n", Ifn);

    int fd = udp_client_creat_con(Ifn, 9999);
    printf("fd = %d\n", fd);

    socklen_t addrlen = sizeof(struct sockaddr);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(19999);
    servaddr.sin_addr.s_addr = inet_addr("192.168.11.146");

    //inet_pton(AF_INET, "192.168.11.146", &servaddr.sin_addr);

    sendto(fd, "hello", 5, 0, (struct sockaddr *)&servaddr, addrlen);
    close(fd);

   /* while ((opt = getopt_long(argc, argv, "", long_opts, &opt_index)) != -1) 
    {
        switch (opt) 
        {
            case 2:
                slotid = atoi(optarg);
                //api_reboot(slotid);
                exit(0);
                break;
            default: 
                exit(0);
        }
    }*/

    return 0;
}
