#include "utils.h"

char *dec(long long x, char *buff)
{
    *--buff = 0;
    if (x < 0)
    {
        *--buff = '-';
        x *= -1;
    }
    if (x == 0)
        *--buff = '0';
    for (; x != 0; x /= 10)
        *--buff = (x % 10) + '0';
    return buff;
}

char *hex(size_t x, char *buff)
{
    char hex_table[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    *--buff = 0;
    if (x == 0)
        *--buff = '0';
    for (; x != 0; x /= 16)
        *--buff = hex_table[x % 16];
    return buff;
}

void *memcpy(void *dest, const void *src, size_t n)
{
    void *tmp = dest;
    for (; n != 0; n--)
        *(char *)dest++ = *(char *)src++;
    return tmp;
}

void *memset(void *s, int c, size_t n)
{
    void *tmp = s;
    for (; n != 0; n--)
        *(char *)s++ = c;
    return tmp;
}