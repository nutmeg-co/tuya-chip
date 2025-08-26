/*
 * Custom TCP transport layer implementation
 * Wrapper around UNIX TCP sockets for network communication
 */

#include "transport_tcp.h"
#include "mbedtls/ssl.h"  /* For MBEDTLS_ERR_SSL_WANT_READ/WRITE */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>

/* Initialize transport context */
void transport_tcp_init(transport_tcp_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    
    ctx->fd = -1;
    ctx->connected = 0;
}

/* Connect to a host:port */
int transport_tcp_connect(transport_tcp_t *ctx, const char *host, const char *port)
{
    struct addrinfo hints, *addr_list, *cur;
    int ret;
    
    if (ctx == NULL || host == NULL || port == NULL) {
        return TRANSPORT_TCP_ERR_INVALID_PARAM;
    }
    
    /* Close existing connection if any */
    if (ctx->fd >= 0) {
        close(ctx->fd);
        ctx->fd = -1;
        ctx->connected = 0;
    }
    
    /* Setup hints for getaddrinfo */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;     /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_protocol = IPPROTO_TCP;
    
    /* Resolve hostname */
    ret = getaddrinfo(host, port, &hints, &addr_list);
    if (ret != 0) {
        fprintf(stderr, "getaddrinfo failed: %s\n", gai_strerror(ret));
        return TRANSPORT_TCP_ERR_UNKNOWN_HOST;
    }
    
    /* Try each address until we successfully connect */
    for (cur = addr_list; cur != NULL; cur = cur->ai_next) {
        ctx->fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (ctx->fd < 0) {
            continue;
        }
        
        if (connect(ctx->fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            /* Connected successfully */
            ctx->connected = 1;
            break;
        }
        
        /* Connection failed, close socket and try next */
        close(ctx->fd);
        ctx->fd = -1;
    }
    
    freeaddrinfo(addr_list);
    
    if (!ctx->connected) {
        return TRANSPORT_TCP_ERR_CONNECT_FAILED;
    }
    
    return TRANSPORT_TCP_OK;
}

/* Send data (compatible with mbedtls bio callback) */
int transport_tcp_send(void *ctx, const unsigned char *buf, size_t len)
{
    transport_tcp_t *tcp_ctx = (transport_tcp_t *)ctx;
    ssize_t ret;
    
    if (tcp_ctx == NULL || tcp_ctx->fd < 0) {
        return TRANSPORT_TCP_ERR_NOT_CONNECTED;
    }
    
    ret = send(tcp_ctx->fd, buf, len, 0);
    
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return MBEDTLS_ERR_SSL_WANT_WRITE;
        }
        
        if (errno == EPIPE || errno == ECONNRESET) {
            return TRANSPORT_TCP_ERR_SEND_FAILED;
        }
        
        return TRANSPORT_TCP_ERR_SEND_FAILED;
    }
    
    return (int)ret;
}

/* Receive data (compatible with mbedtls bio callback) */
int transport_tcp_recv(void *ctx, unsigned char *buf, size_t len)
{
    transport_tcp_t *tcp_ctx = (transport_tcp_t *)ctx;
    ssize_t ret;
    
    if (tcp_ctx == NULL || tcp_ctx->fd < 0) {
        return TRANSPORT_TCP_ERR_NOT_CONNECTED;
    }
    
    ret = recv(tcp_ctx->fd, buf, len, 0);
    
    if (ret < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return MBEDTLS_ERR_SSL_WANT_READ;
        }
        
        if (errno == EPIPE || errno == ECONNRESET) {
            return TRANSPORT_TCP_ERR_RECV_FAILED;
        }
        
        return TRANSPORT_TCP_ERR_RECV_FAILED;
    }
    
    if (ret == 0) {
        /* Connection closed by peer */
        return 0;
    }
    
    return (int)ret;
}

/* Close connection and cleanup */
void transport_tcp_close(transport_tcp_t *ctx)
{
    if (ctx == NULL) {
        return;
    }
    
    if (ctx->fd >= 0) {
        shutdown(ctx->fd, SHUT_RDWR);
        close(ctx->fd);
        ctx->fd = -1;
    }
    
    ctx->connected = 0;
}