/*
 * Custom TCP transport layer
 * Wrapper around UNIX TCP sockets for network communication
 */

#ifndef TRANSPORT_TCP_H
#define TRANSPORT_TCP_H

#include <stddef.h>

/* Error codes */
#define TRANSPORT_TCP_OK                    0
#define TRANSPORT_TCP_ERR_SOCKET_FAILED    -1
#define TRANSPORT_TCP_ERR_CONNECT_FAILED   -2
#define TRANSPORT_TCP_ERR_SEND_FAILED      -3
#define TRANSPORT_TCP_ERR_RECV_FAILED      -4
#define TRANSPORT_TCP_ERR_UNKNOWN_HOST     -5
#define TRANSPORT_TCP_ERR_INVALID_PARAM    -6
#define TRANSPORT_TCP_ERR_NOT_CONNECTED    -7

/* Transport TCP context structure */
typedef struct {
    int fd;                 /* Socket file descriptor */
    int connected;          /* Connection status flag */
} transport_tcp_t;

/* Initialize transport context */
void transport_tcp_init(transport_tcp_t *ctx);

/* Connect to a host:port */
int transport_tcp_connect(transport_tcp_t *ctx, const char *host, const char *port);

/* Send data (compatible with mbedtls bio callback) */
int transport_tcp_send(void *ctx, const unsigned char *buf, size_t len);

/* Receive data (compatible with mbedtls bio callback) */
int transport_tcp_recv(void *ctx, unsigned char *buf, size_t len);

/* Close connection and cleanup */
void transport_tcp_close(transport_tcp_t *ctx);

#endif /* TRANSPORT_TCP_H */