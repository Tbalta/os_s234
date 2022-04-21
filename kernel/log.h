#ifndef LOG_H
#define LOG_H

#include "utils/printf.h"

#define COLOR_RED        "\033[0;31m"
#define COLOR_GREEN      "\033[0;32m"
#define COLOR_YELLOW     "\033[0;33m"
#define COLOR_BLUE       "\033[0;34m"
#define COLOR_MAGENTA    "\033[0;35m"
#define COLOR_CYAN       "\033[0;36m"
#define COLOR_WHITE      "\033[0;37m"
#define COLOR_RESET      "\033[0m"

#define __LOG_INFO  COLOR_BLUE  "[INFO ]" COLOR_RESET
#define __LOG_OK    COLOR_GREEN "[OK   ]" COLOR_RESET
#define __LOG_ERROR COLOR_RED   "[ERROR]" COLOR_RESET


#define S1(x) #x
#define S2(x) S1(x)
#define LOCATION __FILE__ ":" S2(__LINE__)

#define LOG_INFO(fmt, ...) printf(__LOG_INFO "(" LOCATION "): " fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define LOG_OK(fmt, ...) printf(__LOG_OK "(" LOCATION "): " fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define LOG_ERROR(fmt, ...) printf(__LOG_ERROR "(" LOCATION "): " fmt "\n" __VA_OPT__(,) __VA_ARGS__)
#define PANIC(fmt, ...) {LOG_ERROR(fmt __VA_OPT__(,) __VA_ARGS__); PAUSE();}
#define ASSERT(cond, fmt, ...) {if (!(cond)) PANIC(fmt __VA_OPT__(,) __VA_ARGS__);}


#define PAUSE() {while(1) asm volatile ("hlt");}



#endif