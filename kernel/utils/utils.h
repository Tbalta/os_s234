#ifndef UTILS_H
#define UTILS_H
#include <stddef.h>

/**
 * @brief Convert an int64_t to his str representation.
 * 
 * @param x The int to convert.
 * @param buff High address of the buffer. 
 * @return char* The str representation.
 */
char *dec(long long x, char *buff);
char *hex(size_t x, char *buff);

void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *s, int c, size_t n);


#endif