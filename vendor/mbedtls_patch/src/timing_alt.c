/**
 * \file timing_alt.c
 * 
 * \brief Alternative timing implementation for bare metal ARM
 */

#include "mbedtls/timing_alt.h"
#include <stddef.h>

/* Simple counter for timing - increments on each call
 * In a real embedded system, this would use a hardware timer
 * For now, we use a simple counter that increments on each access
 */
static volatile uint64_t timing_counter = 0;

/* Alarm flag (not used in bare metal but required by interface) */
volatile int mbedtls_timing_alarmed = 0;

/**
 * \brief          Get current time in milliseconds
 */
mbedtls_ms_time_t mbedtls_ms_time(void)
{
    /* Return current counter value as milliseconds
     * In a real implementation, this would read a hardware timer
     * For now, increment and return the counter */
    return (mbedtls_ms_time_t)(timing_counter++);
}

/**
 * \brief          Return the CPU cycle counter value
 */
unsigned long mbedtls_timing_hardclock(void)
{
    /* In a real implementation, this would read a hardware counter
     * For now, return an incrementing value */
    return (unsigned long)(timing_counter++);
}

/**
 * \brief          Return elapsed time in milliseconds
 */
unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val, int reset)
{
    unsigned long current = mbedtls_timing_hardclock();
    
    if (reset) {
        val->value = current;
        return 0;
    }
    
    /* Return elapsed time (assumes 1 tick = 1 ms for simplicity)
     * In real implementation, this would convert hardware timer ticks to ms */
    return (unsigned long)(current - val->value);
}

/**
 * \brief          Setup an alarm clock
 * 
 * \note           Not implemented for bare metal systems
 */
void mbedtls_set_alarm(int seconds)
{
    /* Alarm functionality not supported on bare metal
     * This is primarily used for test programs */
    (void)seconds;
    mbedtls_timing_alarmed = 0;
}

/**
 * \brief          Set a delay
 */
void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *)data;
    
    if (ctx == NULL) {
        return;
    }
    
    ctx->int_ms = int_ms;
    ctx->fin_ms = fin_ms;
    
    /* Reset timer if final delay is non-zero */
    if (fin_ms != 0) {
        mbedtls_timing_get_timer(&ctx->timer, 1);
    }
}

/**
 * \brief          Get the delay status
 */
int mbedtls_timing_get_delay(void *data)
{
    mbedtls_timing_delay_context *ctx = (mbedtls_timing_delay_context *)data;
    unsigned long elapsed;
    
    if (ctx == NULL) {
        return -1;
    }
    
    /* If no delay was set, return cancelled status */
    if (ctx->fin_ms == 0) {
        return -1;
    }
    
    /* Get elapsed time since delay was set */
    elapsed = mbedtls_timing_get_timer(&ctx->timer, 0);
    
    /* Check if final delay has expired */
    if (elapsed >= ctx->fin_ms) {
        return 2;
    }
    
    /* Check if intermediate delay has expired */
    if (ctx->int_ms > 0 && elapsed >= ctx->int_ms) {
        return 1;
    }
    
    /* No delay has expired yet */
    return 0;
}

#ifdef MBEDTLS_SELF_TEST
/* Self-test functionality can be added here if needed */
int mbedtls_timing_self_test(int verbose)
{
    (void)verbose;
    /* Basic self-test always passes for now */
    return 0;
}
#endif /* MBEDTLS_SELF_TEST */