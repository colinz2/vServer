#ifndef _BS_LOG_H
#define _BS_LOG_H 

#include <stdio.h>

void log_open(const char *log_path, int flag);
void log_close(void);
void log_print(const char *format, ...);

#endif