#ifndef _BS_MAIN_
#define _BS_MAIN_

#include <stdio.h>

typedef struct sockaddr_in  SA;
typedef struct in_addr      IA;

#ifdef __GNUC__
#define likely(x)       (__builtin_expect(!!(x), 1))
#define unlikely(x)     (__builtin_expect(!!(x), 0))
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif


static inline void 
swap_array(unsigned char *a, unsigned char *b, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        *a = *a ^ *b;
        *b = *a ^ *b;
        *a = *a ^ *b;
        a++; b++;
    }
}

#define console_print(fmt, args...) \
        do { \
            fprintf(stdout, fmt"", ##args); \
            fflush(stdout); \
        } while (0)

#define CONSOLE_CLEAR       "\033[2J\033[0;0H"
#define CONSOLE_BACK_N(n)   "\033["#n"A"

#endif

