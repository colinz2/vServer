#ifndef _BS_UTILS_
#define _BS_UTILS_ 

#include <stdio.h>
#include <net/if.h>

int if_not_exist(const char *ifn);
int get_if_index(int fd, struct ifreq *ifr);
int bind_socket_if(int fd, struct ifreq *ifr);
int get_if_macaddr(int fd, struct ifreq *ifr, char *mac, int mac_len) ;
int set_if_promisc(int fd, struct ifreq *ifr, int ture);
int set_blocking(int sd);
int set_nonblocking(int sd);
int set_reuseaddr(int sd);
int set_sndbuf(int sd, int size);
int set_rcvbuf(int sd, int size);
int get_sndbuf(int sd);
int get_rcvbuf(int sd);
int get_io_buff_len(int fd);

ssize_t                     /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n);
ssize_t                     /* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n);
ssize_t                     /* Read "n" bytes from a descriptor. */
reads(int fd, void *vptr, size_t n);

int lock_file_init(const char *daemon_name);

#endif