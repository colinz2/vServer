#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/*
#ip address         arp    imcp   snmp
#192.168.11.1        0       1       1
#1.1.1.1             1       1       1
#192.168.11.2        1       1       1
192.168.11.147       1       1       1
192.168.11.146       1       1       1
192.168.11.204       1       1       1
192.168.55.128       1       1       1
*/

int main(int argc, char *argv[])
{
    int i = 0, j = 0;

    FILE * f = fopen("ip_list", "w");
    if (NULL == f) {
        return -1;
    }

    for (j = 0; j < 8; j++ ) {
         for (i = 1; i < 255; i++) {
            fprintf(f, "2.2.%d.%d    1 1 1\n",j, i);
         }   
    }
    fclose(f);
    return 0;
}