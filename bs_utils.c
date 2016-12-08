#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/file.h>
#include <ifaddrs.h>
#include <dirent.h>

int 
if_not_exist(const char *ifn)
{
    struct ifaddrs *ifaddr, *ifa;
    int  n, ret;

    if (getifaddrs(&ifaddr) == -1) {
        printf("%s", strerror(errno));
        return -1;
    }

    for (ifa = ifaddr, ret = 1; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL || ifa->ifa_name == NULL)
            continue;
        if (strcmp(ifa->ifa_name, ifn) == 0) {
            ret = 0;
            break;
        }
    }

    freeifaddrs(ifaddr);
    return ret;
}

int 
get_if_index(int fd, struct ifreq *ifr)
{
    if (ioctl(fd, SIOCGIFINDEX, ifr) < 0) {
        printf("ioctl:%s\n", strerror(errno));
        return -1;
    }
    return 0;
}

int 
bind_socket_if(int fd, struct ifreq *ifr)
{
    if (get_if_index(fd, ifr) < 0) {
        return -1;
    } 
    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, ifr, sizeof(*ifr)) < 0) {
        printf("%s\n", "setsockopt");
        return -1;
    }
    return 0;
}

int 
get_if_macaddr(int fd, struct ifreq *ifr, char *mac, int mac_len) 
{ 
    char *ptr;    
    int i = 0;

    if (mac == NULL) {
        return -1;
    }
    if (ioctl(fd, SIOCGIFHWADDR, ifr) < 0 ) {
        perror("Dev_GetIfMacAddr, ioctl");
        return -1;
    } 
    memcpy(mac, ifr->ifr_hwaddr.sa_data, 6);
    return 0;
} 

int 
set_if_promisc(int fd, struct ifreq *ifr, int ture)  
{  
    if (ioctl(fd, SIOCGIFFLAGS, ifr) < 0) {  
        printf("%s", strerror(errno));  
        return -1;    
    }
    if (ture) {
        ifr->ifr_flags |= IFF_PROMISC; 
    } else {
        ifr->ifr_flags &= ~IFF_PROMISC;
    }
    
    if (ioctl(fd, SIOCSIFFLAGS, ifr) < 0) {  
        printf("%s", strerror(errno));   
        return -1;    
    }  
    return 0;  
} 

int
set_blocking(int sd)
{
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags & ~O_NONBLOCK);
}

int
set_nonblocking(int sd)
{
    int flags;

    flags = fcntl(sd, F_GETFL, 0);
    if (flags < 0) {
        return flags;
    }

    return fcntl(sd, F_SETFL, flags | O_NONBLOCK);
}

int 
get_io_buff_len(int fd)
{
    int len = 0;
    ioctl(fd, FIONREAD, &len);
    return len;
}

int
set_reuseaddr(int sd)
{
    int reuse;
    socklen_t len;

    reuse = 1;
    len = sizeof(reuse);

    return setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &reuse, len);
}

int
set_sndbuf(int sd, int size)
{
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, /*SO_SNDBUF*/ SO_SNDBUFFORCE, &size, len);
}

int
set_rcvbuf(int sd, int size)
{
    socklen_t len;

    len = sizeof(size);

    return setsockopt(sd, SOL_SOCKET, /*SO_RCVBUF*/ SO_RCVBUFFORCE, &size, len);
}

int
get_soerror(int sd)
{
    int status, err;
    socklen_t len;

    err = 0;
    len = sizeof(err);

    status = getsockopt(sd, SOL_SOCKET, SO_ERROR, &err, &len);
    if (status == 0) {
        errno = err;
    }

    return status;
}

int
get_sndbuf(int sd)
{
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_SNDBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}

int
get_rcvbuf(int sd)
{
    int status, size;
    socklen_t len;

    size = 0;
    len = sizeof(size);

    status = getsockopt(sd, SOL_SOCKET, SO_RCVBUF, &size, &len);
    if (status < 0) {
        return status;
    }

    return size;
}

ssize_t                     /* Write "n" bytes to a descriptor. */
writen(int fd, const void *vptr, size_t n)
{
    size_t      nleft;
    ssize_t     nwritten;
    const char  *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nwritten = write(fd, ptr, nleft)) <= 0) {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;       /* and call write() again */
            else
                return(-1);         /* error */
        }

        nleft -= nwritten;
        ptr   += nwritten;
    }
    return(n);
}


ssize_t                     /* Read "n" bytes from a descriptor. */
readn(int fd, void *vptr, size_t n)
{
    size_t  nleft;
    ssize_t nread;
    char    *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0) {
        if ( (nread = read(fd, ptr, nleft)) < 0) {
            if (errno == EINTR)
                nread = 0;      /* and call read() again */
            else
                return(-1);
        } else if (nread == 0)
            break;              /* EOF */

        nleft -= nread;
        ptr   += nread;
    }
    return(n - nleft);      /* return >= 0 */
}

ssize_t                     /* Read "n" bytes from a descriptor. */
reads(int fd, void *vptr, size_t n)
{
    size_t  nleft;
    ssize_t nread;
    char    *ptr;

    ptr = vptr;
    nleft = n;

    do {
        nread = read(fd, ptr, nleft);
        if (nread < 0 && errno != EINTR && errno != EAGAIN) {
            return nread;
        }
        nleft -= nread;
        ptr   += nread;      
    }
    while ((n - nleft) && (errno == EINTR || errno == EAGAIN ));
    return(n - nleft);      /* return >= 0 */
}

static char *
get_process_name(void)
{
    static char process_name[32];
    char buf[256] = {0};
    char *p;
    int r = readlink("/proc/self/exe", buf, sizeof(buf));
    if (r == -1) {
        perror("readlink");
        return NULL;
    }
    p = strrchr(buf, '/');
    if (p) {
        snprintf(process_name, sizeof(process_name), "%s", p + 1);
        return process_name;
    } else {
        return NULL;
    }
}

int
check_pid(const char *daemon_name) 
{
    int pid = 0;
    char path[256] = {0};

    snprintf(path, sizeof(path), "/tmp/pid__%s.pid", daemon_name);
    FILE *f = fopen(path, "r");
    if (f == NULL)
        return 0;
    int n = fscanf(f, "%d", &pid);
    fclose(f);

    if (n !=1 || pid == 0 || pid == getpid()) {
        return 0;
    }

    if (kill(pid, 0) && errno == ESRCH)
        return 0;

    return pid;
}

static int
pid_output(const char *daemon_name) 
{
    FILE *f;
    int pid = 0;
    char path[256] = {0};
    snprintf(path, sizeof(path), "/tmp/pid__%s.pid", daemon_name);
    int fd = open(path, O_RDWR|O_CREAT, 0644);
    if (fd == -1) {
        fprintf(stderr, "can't create %s.\n", path);
        return 0;
    }
    f = fdopen(fd, "r+");
    if (f == NULL) {
        fprintf(stderr, "can't open %s.\n", path);
        return 0;
    }

    if (flock(fd, LOCK_EX | LOCK_NB) == -1) {
        int n = fscanf(f, "%d", &pid);
        fclose(f);
        if (n != 1) {
            fprintf(stderr, "can't lock and read path.\n");
        } else {
            fprintf(stderr, "can't lock path, lock is held by pid %d.\n", pid);
        }
        return 0;
    }

    pid = getpid();
    if (!fprintf(f,"%d\n", pid)) {
        fprintf(stderr, "can't write pid.\n");
        close(fd);
        return 0;
    }
    fflush(f);

    return pid;
}

int
lock_file_init(const char *daemon_name)
{
    int pid = check_pid(daemon_name);

    if (pid) {
        return 1;
    }
    pid = pid_output(daemon_name);
    if (pid == 0) {
        return 1;
    }
    return 0;
}