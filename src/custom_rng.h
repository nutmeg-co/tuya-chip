/*
 * Custom Random Number Generator
 * Alternative RNG implementation for mbedtls
 */

#ifndef CUSTOM_RNG_H
#define CUSTOM_RNG_H

#include <stddef.h>

/* Custom RNG context structure */
typedef struct {
    int initialized;
    unsigned int seed;
    /* Add more fields as needed for your RNG implementation */
} custom_rng_context;

/* Initialize custom RNG */
int custom_rng_init(custom_rng_context *ctx);

/* Seed the custom RNG */
int custom_rng_seed(custom_rng_context *ctx, const unsigned char *seed_data, size_t seed_len);

/* Generate random data (compatible with mbedtls callback) */
int custom_rng_random(void *ctx, unsigned char *output, size_t len);

/* Free custom RNG resources */
void custom_rng_free(custom_rng_context *ctx);

#endif /* CUSTOM_RNG_H */