/*
 * Custom Random Number Generator Implementation
 * This is a simple example - replace with your actual RNG implementation
 */

#include "custom_rng.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>

/* Initialize custom RNG */
int custom_rng_init(custom_rng_context *ctx)
{
    if (ctx == NULL) {
        return -1;
    }
    
    ctx->initialized = 0;
    ctx->seed = 0;
    
    return 0;
}

/* Seed the custom RNG */
int custom_rng_seed(custom_rng_context *ctx, const unsigned char *seed_data, size_t seed_len)
{
    if (ctx == NULL) {
        return -1;
    }
    
    /* Simple seeding - combine seed data into a single value */
    /* In production, use a proper seeding mechanism */
    unsigned int seed = (unsigned int)time(NULL);
    
    if (seed_data != NULL && seed_len > 0) {
        for (size_t i = 0; i < seed_len; i++) {
            seed = seed * 31 + seed_data[i];
        }
    }
    
    ctx->seed = seed;
    ctx->initialized = 1;
    
    return 0;
}

/* Generate random data using /dev/urandom (Unix/Linux) */
int custom_rng_random(void *ctx, unsigned char *output, size_t len)
{
    custom_rng_context *rng_ctx = (custom_rng_context *)ctx;
    
    if (rng_ctx == NULL || output == NULL || len == 0) {
        return -1;
    }
    
    if (!rng_ctx->initialized) {
        return -1;
    }
    
#ifdef __unix__
    /* Use /dev/urandom for Unix/Linux systems */
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        /* Fallback to weak random if /dev/urandom not available */
        for (size_t i = 0; i < len; i++) {
            output[i] = (unsigned char)(rand() & 0xFF);
        }
        return 0;
    }
    
    ssize_t bytes_read = read(fd, output, len);
    close(fd);
    
    if (bytes_read != (ssize_t)len) {
        return -1;
    }
#else
    /* Simple fallback for non-Unix systems */
    /* WARNING: This is not cryptographically secure! */
    srand(rng_ctx->seed);
    for (size_t i = 0; i < len; i++) {
        output[i] = (unsigned char)(rand() & 0xFF);
        rng_ctx->seed = rng_ctx->seed * 1103515245 + 12345; /* LCG */
    }
#endif
    
    return 0;
}

/* Free custom RNG resources */
void custom_rng_free(custom_rng_context *ctx)
{
    if (ctx == NULL) {
        return;
    }
    
    ctx->initialized = 0;
    ctx->seed = 0;
    /* Clean up any additional resources here */
}