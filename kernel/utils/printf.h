#ifndef PRINTF_H
#define PRINTF_H


#include <stddef.h>
#include <stdarg.h>


int vsnprintf(char *str, size_t n, const char *format, va_list arguments);
void printf(const char *fmt, ...);

#endif