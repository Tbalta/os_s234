#ifndef BITS_H
#define BITS_H

#define MASK(n) ((1 << (n)) - 1)
#define TAKE_LEFT(x, n) ((x) & MASK(n))
#define ALIGN_LOWER(number, alignment) ((number) - ((number) % (alignment)))
#define ALIGN_UPPER(number, alignment) (ALIGN_LOWER((number) + (alignment) - 1, alignment))


#endif