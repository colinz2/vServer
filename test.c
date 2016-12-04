#include <stdio.h>


void 
swap_array(char *a, char *b, int len)
{
    int i;
    for (i = 0; i < len; i++) {
        *a = *a ^ *b;
        *b = *a ^ *b;
        *a = *a ^ *b;
        a++;
        b++;
    }      
}


int main(int argc, char const *argv[])
{
    char a[16] = {"abc"};
    char b[16] = {"123"};

    swap_array(a, b, 4);
    printf("%s\n", a);
    printf("%s\n", b);
    return 0;
}