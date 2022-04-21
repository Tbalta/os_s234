#include "pit.h"
#include "../port.h"
#include "../log.h"
#include "../pic/pic.h"
#define TIMER_FREQUENCY 1193180

#include <stdint.h>

void init_timer(int hz)
{
    uint64_t divisor = TIMER_FREQUENCY / hz;

    outb(0x43, 0x36);           /* Set our command byte 0x36 */
    outb(0x40, divisor & 0xFF); /* Set low byte of divisor */
    outb(0x40, divisor >> 8);   /* Set high byte of divisor */
}

/* This will keep track of how many ticks that the system
 *  has been running for */
size_t timer_ticks = 0;

static timer_func_t timer_funcs[10] = {0};
static size_t timer_func_count = 0;

/* Handles the timer. In this case, it's very simple: We
 *  increment the 'timer_ticks' variable every time the
 *  timer fires. By default, the timer fires 18.222 times
 *  per second. Why 18.222Hz? Some engineer at IBM must've
 *  been smoking something funky */
void timer_handler(stack_frame_t *stack_frame)
{
    PIC_sendEOI(stack_frame->int_no);

    for (size_t i = 0; i < timer_func_count; i++)
    {
        if (timer_ticks % timer_funcs[i].tick_to_wait == 0)
        {
            LOG_INFO("timer_handler");
            timer_funcs[i].func(stack_frame);
        }
    }
    timer_ticks++;
    // LOG_INFO("timer tick %d", timer_ticks);
}

/* Sets up the system clock by installing the timer handler
 *  into IRQ0 */
void timer_install()
{
    /* Installs 'timer_handler' to IRQ0 */
    register_interrupt_handler(32, &timer_handler);
}

void timer_add_func_hz(void (*func)(stack_frame_t *stack_frame), int hz)
{
    timer_add_func_tick(func, TIMER_FREQUENCY / hz);
}


void timer_add_func_tick(void (*func)(stack_frame_t *stack_frame), size_t tick)
{
    ASSERT(timer_func_count < 10, "Too many timer functions");
    timer_funcs[timer_func_count++] = (timer_func_t){func, tick};
    LOG_OK("timer function added {.func: 0x%x, .tick_to_wait: %d}", timer_funcs[timer_func_count - 1].func, timer_funcs[timer_func_count - 1].tick_to_wait);
    LOG_INFO("timer_func_count: %d", timer_func_count);
}
