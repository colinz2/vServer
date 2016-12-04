#ifndef _BS_UTILS_
#define _BS_UTILS_ 
#include <stdio.h>
int if_not_exist(const char *ifn);
int bind_socket_if(int fd, const char *ifn);
int set_if_promisc(const char *ifn, int ture);
int set_blocking(int sd);
int set_nonblocking(int sd);
int nc_set_reuseaddr(int sd);
int set_sndbuf(int sd, int size);
int set_rcvbuf(int sd, int size);
int get_sndbuf(int sd);
int get_rcvbuf(int sd);
int get_if_macaddr(int fd, const char *ifn, char *mac, int mac_len);

ssize_t                     /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n);
ssize_t                     /* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n);
ssize_t                     /* Read "n" bytes from a descriptor. */
reads(int fd, void *vptr, size_t n);

#endif