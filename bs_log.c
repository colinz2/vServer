#include "bs_log.h"
#include "bs_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

static FILE *log_file = NULL;
void 
log_open(const char *log_path, int flag)
{
    if (flag == 1) {
        log_file = fopen(log_path, "w");
    }
}

void
log_close(void)
{
    if (log_file) {
        fclose(log_file);
    }
}

void
log_print(const char *format, ...)
{
    FILE *f;
    va_list args;
    va_start(args, format);
    if (likely(log_file != NULL)) {
        vfprintf(log_file, format, args);
        fflush(log_file);
    }
    va_end(args);
}

