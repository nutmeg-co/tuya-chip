/*
 * Simple HTTPS client to connect to laundrygo.id API
 * Without certificate verification (for testing purposes only)
 */

/* Uncomment to use custom RNG instead of CTR_DRBG */
#define CUSTOM_RNG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "transport_tcp.h"
#include "mbedtls/ssl.h"
#include "mbedtls/error.h"
#include "mbedtls/debug.h"

#ifdef CUSTOM_RNG
#include "custom_rng.h"
#else
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#endif

#define SERVER_HOST "laundrygo.id"
#define SERVER_PORT "443"
#define GET_REQUEST "GET /api HTTP/1.1\r\n" \
                    "Host: laundrygo.id\r\n" \
                    "User-Agent: mbedtls-client/1.0\r\n" \
                    "Connection: close\r\n" \
                    "\r\n"

#define DEBUG_LEVEL 1

static void my_debug(void *ctx, int level,
                     const char *file, int line,
                     const char *str)
{
    ((void) level);
    fprintf((FILE *) ctx, "%s:%04d: %s", file, line, str);
    fflush((FILE *) ctx);
}

int main(int argc, char *argv[])
{
    int ret = 1, len;
    transport_tcp_t transport;
    unsigned char buf[4096];
    const char *pers = "tuya_client";

#ifdef CUSTOM_RNG
    custom_rng_context custom_rng;
#else
    mbedtls_entropy_context entropy;
    mbedtls_ctr_drbg_context ctr_drbg;
#endif
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config conf;

#if defined(MBEDTLS_DEBUG_C)
    mbedtls_debug_set_threshold(DEBUG_LEVEL);
#endif

    /* Initialize contexts */
    transport_tcp_init(&transport);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&conf);
    
#ifdef CUSTOM_RNG
    custom_rng_init(&custom_rng);
#else
    mbedtls_ctr_drbg_init(&ctr_drbg);
    mbedtls_entropy_init(&entropy);
#endif

    printf("\n==== HTTPS Client for laundrygo.id ====\n\n");

    /* 1. Initialize the RNG */
#ifdef CUSTOM_RNG
    printf("  . Initializing custom RNG...");
#else
    printf("  . Seeding the random number generator...");
#endif
    fflush(stdout);

#ifdef CUSTOM_RNG
    if ((ret = custom_rng_seed(&custom_rng, (const unsigned char *)pers, strlen(pers))) != 0) {
        printf(" failed\n  ! custom_rng_seed returned %d\n", ret);
        goto exit;
    }
#else
    if ((ret = mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy,
                                     (const unsigned char *) pers,
                                     strlen(pers))) != 0) {
        printf(" failed\n  ! mbedtls_ctr_drbg_seed returned %d\n", ret);
        goto exit;
    }
#endif

    printf(" ok\n");

    /* 2. Connect to server */
    printf("  . Connecting to tcp/%s/%s...", SERVER_HOST, SERVER_PORT);
    fflush(stdout);

    if ((ret = transport_tcp_connect(&transport, SERVER_HOST, SERVER_PORT)) != 0) {
        printf(" failed\n  ! transport_tcp_connect returned %d\n\n", ret);
        goto exit;
    }

    printf(" ok\n");

    /* 3. Setup SSL/TLS structure */
    printf("  . Setting up the SSL/TLS structure...");
    fflush(stdout);

    if ((ret = mbedtls_ssl_config_defaults(&conf,
                                           MBEDTLS_SSL_IS_CLIENT,
                                           MBEDTLS_SSL_TRANSPORT_STREAM,
                                           MBEDTLS_SSL_PRESET_DEFAULT)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_config_defaults returned %d\n\n", ret);
        goto exit;
    }

    printf(" ok\n");

    /* IMPORTANT: Skip certificate verification - NOT SECURE, for testing only */
    printf("  . Configuring SSL/TLS (skipping certificate verification)...");
    mbedtls_ssl_conf_authmode(&conf, MBEDTLS_SSL_VERIFY_NONE);
    
#ifdef CUSTOM_RNG
    mbedtls_ssl_conf_rng(&conf, custom_rng_random, &custom_rng);
#else
    mbedtls_ssl_conf_rng(&conf, mbedtls_ctr_drbg_random, &ctr_drbg);
#endif
    
    mbedtls_ssl_conf_dbg(&conf, my_debug, stdout);

    if ((ret = mbedtls_ssl_setup(&ssl, &conf)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_setup returned %d\n\n", ret);
        goto exit;
    }

    if ((ret = mbedtls_ssl_set_hostname(&ssl, SERVER_HOST)) != 0) {
        printf(" failed\n  ! mbedtls_ssl_set_hostname returned %d\n\n", ret);
        goto exit;
    }

    mbedtls_ssl_set_bio(&ssl, &transport, transport_tcp_send, transport_tcp_recv, NULL);

    printf(" ok\n");

    /* 4. Perform SSL/TLS handshake */
    printf("  . Performing the SSL/TLS handshake...");
    fflush(stdout);

    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            char error_buf[100];
            mbedtls_strerror(ret, error_buf, 100);
            printf(" failed\n  ! mbedtls_ssl_handshake returned -0x%x: %s\n\n",
                   (unsigned int) -ret, error_buf);
            goto exit;
        }
    }

    printf(" ok\n");
    printf("    [ Protocol is %s ]\n", mbedtls_ssl_get_version(&ssl));
    printf("    [ Ciphersuite is %s ]\n", mbedtls_ssl_get_ciphersuite(&ssl));

    /* 5. Send HTTP GET request */
    printf("\n  > Write to server:");
    fflush(stdout);

    len = sprintf((char *) buf, GET_REQUEST);

    while ((ret = mbedtls_ssl_write(&ssl, buf, len)) <= 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            printf(" failed\n  ! mbedtls_ssl_write returned %d\n\n", ret);
            goto exit;
        }
    }

    len = ret;
    printf(" %d bytes written\n\n", len);
    printf("Request sent:\n%s\n", (char *) buf);

    /* 6. Read HTTP response */
    printf("  < Read from server:\n\n");
    fflush(stdout);

    do {
        len = sizeof(buf) - 1;
        memset(buf, 0, sizeof(buf));
        ret = mbedtls_ssl_read(&ssl, buf, len);

        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) {
            continue;
        }

        if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) {
            printf("\nConnection closed by server\n");
            ret = 0;
            break;
        }

        if (ret < 0) {
            printf("failed\n  ! mbedtls_ssl_read returned %d\n\n", ret);
            break;
        }

        if (ret == 0) {
            printf("\nEOF\n\n");
            break;
        }

        len = ret;
        printf("%.*s", len, (char *) buf);
    } while (1);

    printf("\n");
    mbedtls_ssl_close_notify(&ssl);

exit:
#ifdef MBEDTLS_ERROR_C
    if (ret != 0) {
        char error_buf[100];
        mbedtls_strerror(ret, error_buf, 100);
        printf("Last error was: -0x%X - %s\n\n", (unsigned int) -ret, error_buf);
    }
#endif

    /* Cleanup */
    transport_tcp_close(&transport);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&conf);
    
#ifdef CUSTOM_RNG
    custom_rng_free(&custom_rng);
#else
    mbedtls_ctr_drbg_free(&ctr_drbg);
    mbedtls_entropy_free(&entropy);
#endif

    printf("==== End of HTTPS Client ====\n\n");

    return ret != 0 ? EXIT_FAILURE : EXIT_SUCCESS;
}