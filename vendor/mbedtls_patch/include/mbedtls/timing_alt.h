/**
 * \file timing_alt.h
 * 
 * \brief Alternative timing implementation for bare metal ARM
 *
 * This provides custom timing functions for embedded systems
 * that don't have standard POSIX timing functions.
 */
#ifndef MBEDTLS_TIMING_ALT_H
#define MBEDTLS_TIMING_ALT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \brief          timer structure
 */
struct mbedtls_timing_hr_time {
    uint64_t value;
};

/**
 * \brief          Context for mbedtls_timing_set/get_delay()
 */
typedef struct mbedtls_timing_delay_context {
    struct mbedtls_timing_hr_time timer;
    uint32_t int_ms;
    uint32_t fin_ms;
} mbedtls_timing_delay_context;

/**
 * \brief          Get the current hardclock counter value
 *
 * \return         Current hardclock counter value
 */
unsigned long mbedtls_timing_hardclock(void);

/**
 * \brief          Return elapsed time in milliseconds
 *
 * \param val      Timer structure
 * \param reset    If 1, reset the timer; if 0, return elapsed time
 *
 * \return         Elapsed time in milliseconds
 */
unsigned long mbedtls_timing_get_timer(struct mbedtls_timing_hr_time *val, int reset);

/**
 * \brief          Setup an alarm clock
 *
 * \param seconds  Delay before the "mbedtls_timing_alarmed" flag is set
 *
 * \warning        Not implemented for bare metal. This function does nothing.
 */
void mbedtls_set_alarm(int seconds);

/**
 * \brief          Set a delay
 *
 * \param data     Pointer to delay context
 * \param int_ms   Intermediate delay in milliseconds
 * \param fin_ms   Final delay in milliseconds (must be >= int_ms)
 */
void mbedtls_timing_set_delay(void *data, uint32_t int_ms, uint32_t fin_ms);

/**
 * \brief          Get the delay status
 *
 * \param data     Pointer to delay context
 *
 * \return         -1 if cancelled
 *                 0 if none of the delays are expired
 *                 1 if the intermediate delay only is expired
 *                 2 if the final delay is expired
 */
int mbedtls_timing_get_delay(void *data);

#ifdef __cplusplus
}
#endif

#endif /* MBEDTLS_TIMING_ALT_H */