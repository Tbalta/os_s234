#include "printf.h"

#include "utils.h"
#include "serial.h"


int vsnprintf(char *str, size_t n, const char *format, va_list arguments)
{
    // va_list arguments;
    // va_start(arguments, format);
    if (n == 0)
        return 0;
    size_t wrote = 0;
    for (int i = 0; wrote + 1 < n && format[i]; i++)
    {
        if (format[i] != '%')
        {
            str[wrote] = format[i];
            wrote++;
            continue;
        }
        switch (format[i + 1])
        {
        case 'd':
        {
            char buffer[21] = {0};
            int val = (int)va_arg(arguments, int);
            char* result = dec(val, buffer + 21);
            for (int j = 0; result[j] != '\0' && wrote < n - 1; wrote++, j++)
                str[wrote] = result[j];
        }
        break;
        case 'x':
        {
            char buffer[21] = {0};
            size_t val = (size_t)va_arg(arguments, size_t);
            char* result = hex(val, buffer + 21);
            for (int j = 0; result[j] != '\0' && wrote < n - 1; wrote++, j++)
                str[wrote] = result[j];
        }
        break;
        case 's':
        {
            char *val = (char *)va_arg(arguments, char *);
            for (int j = 0; val[j] != '\0' && wrote < n - 1; wrote++, j++)
                str[wrote] = val[j];
        }
        break;
        default:
            str[wrote++] = format[i + 1];
            break;
        }
        i +=1;
    }

    // va_end(arguments);
    str[wrote] = '\0';
    return wrote;
}

void printf(const char *fmt, ...)
{
    char buffer[500];
    va_list args;
    va_start(args, fmt);

    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    serial_send_msg(buffer);
}
