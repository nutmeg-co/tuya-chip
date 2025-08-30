#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <websocket_parser.h>

#define BUFFER_SIZE 4096
#define HTTP_HOST "laundrygo.id"
#define WS_HOST "do.laundrygo.id"
#define WS_PORT "15774"

#define PING_JSON "{\"type\":\"ping\"}"

static int sockfd = -1;
static websocket_parser parser;
static websocket_parser_settings settings;
static const char *ws_path = "/";
static const char *auth_token = NULL;

static int on_frame_header(websocket_parser *p)
{
    printf("Frame header received: opcode=%d, final=%d, mask=%d\n",
           websocket_parser_get_opcode(p),
           websocket_parser_has_final(p),
           websocket_parser_has_mask(p));
    return 0;
}

static int on_frame_body(websocket_parser *p, const char *data, size_t len)
{
    int opcode = websocket_parser_get_opcode(p);

    switch (opcode)
    {
    case WS_OP_TEXT:
        printf("Text message: %.*s\n", (int)len, data);
        break;
    case WS_OP_BINARY:
        printf("Binary message (%zu bytes)\n", len);
        break;
    case WS_OP_PING:
        printf("Ping received\n");
        break;
    case WS_OP_PONG:
        printf("Pong received\n");
        break;
    case WS_OP_CLOSE:
        printf("Close frame received\n");
        break;
    default:
        printf("Unknown opcode: %d\n", opcode);
    }
    return 0;
}

static int on_frame_end(websocket_parser *p)
{
    printf("Frame end\n");

    int opcode = websocket_parser_get_opcode(p);
    if (opcode == WS_OP_PING)
    {
        printf("Sending pong response\n");
        char pong_frame[10];
        size_t frame_len = websocket_build_frame(pong_frame, WS_OP_PONG | WS_FIN, NULL, NULL, 0);
        send(sockfd, pong_frame, frame_len, 0);
    }

    return 0;
}

static int read_line(int fd, char *buffer, size_t max_len)
{
    size_t pos = 0;
    char c;
    ssize_t n;

    while (pos < max_len - 1)
    {
        n = recv(fd, &c, 1, 0);

        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }

        if (n == 0)
        {
            if (pos == 0)
                return 0;
            break;
        }

        buffer[pos++] = c;

        if (c == '\n')
        {
            if (pos >= 2 && buffer[pos - 2] == '\r')
            {
                buffer[pos - 2] = '\0';
                return pos - 2;
            }
            else
            {
                buffer[pos - 1] = '\0';
                return pos - 1;
            }
        }
    }

    buffer[pos] = '\0';
    return pos;
}

static int connect_to_server(const char *host, const char *port)
{
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            perror("socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1)
        {
            close(sockfd);
            perror("connect");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL)
    {
        fprintf(stderr, "Failed to connect\n");
        return -1;
    }

    printf("Connected to %s:%s\n", host, port);
    return 0;
}

static int perform_handshake()
{
    char request[4096];
    char line[1024];
    ssize_t bytes_sent;
    int line_len;
    int status_code = 0;
    int headers_complete = 0;

    const char *ws_key = "dGhlIHNhbXBsZSBub25jZQ==";

    int offset = snprintf(
        request, sizeof(request),
        "GET %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n",
        ws_path, HTTP_HOST, ws_key);

    if (auth_token)
    {
        offset += snprintf(
            request + offset, sizeof(request) - offset,
            "Authorization: Bearer %s\r\n", auth_token);
    }

    snprintf(request + offset, sizeof(request) - offset, "\r\n");

    printf("Sending handshake:\n%s", request);

    bytes_sent = send(sockfd, request, strlen(request), 0);
    if (bytes_sent < 0)
    {
        perror("send");
        return -1;
    }

    printf("Handshake response:\n");

    line_len = read_line(sockfd, line, sizeof(line));
    if (line_len <= 0)
    {
        fprintf(stderr, "Failed to read status line\n");
        return -1;
    }

    printf("%s\n", line);

    if (sscanf(line, "HTTP/1.1 %d", &status_code) != 1)
    {
        fprintf(stderr, "Invalid HTTP response\n");
        return -1;
    }

    while (!headers_complete)
    {
        line_len = read_line(sockfd, line, sizeof(line));
        if (line_len < 0)
        {
            fprintf(stderr, "Failed to read header line\n");
            return -1;
        }

        if (line_len == 0)
        {
            headers_complete = 1;
            printf("\n");
            break;
        }

        printf("%s\n", line);
    }

    if (status_code != 101)
    {
        fprintf(stderr, "WebSocket handshake failed with status code: %d\n", status_code);
        return -1;
    }

    printf("WebSocket handshake successful\n");
    return 0;
}

static int send_text_message(const char *message)
{
    size_t msg_len = strlen(message);
    size_t frame_size = websocket_calc_frame_size(WS_OP_TEXT | WS_FIN | WS_HAS_MASK, msg_len);
    char *frame = malloc(frame_size);

    if (!frame)
    {
        fprintf(stderr, "Failed to allocate frame buffer\n");
        return -1;
    }

    char mask[4] = {0x12, 0x34, 0x56, 0x78};

    size_t frame_len = websocket_build_frame(frame, WS_OP_TEXT | WS_FIN | WS_HAS_MASK, mask, message, msg_len);

    ssize_t sent = send(sockfd, frame, frame_len, 0);
    free(frame);

    if (sent < 0)
    {
        perror("send");
        return -1;
    }

    printf("Sent text message: %s\n", message);
    return 0;
}

static void send_close_frame()
{
    char close_frame[10];
    size_t frame_len = websocket_build_frame(close_frame, WS_OP_CLOSE | WS_FIN, NULL, NULL, 0);
    send(sockfd, close_frame, frame_len, 0);
    printf("Sent close frame\n");
}

static void print_usage(const char *prog_name)
{
    fprintf(stderr, "Usage: %s <path> <bearer_token>\n", prog_name);
    fprintf(stderr, "Example: %s /api/v1/stream abc123token456\n", prog_name);
    fprintf(stderr, "\n");
    fprintf(stderr, "Arguments:\n");
    fprintf(stderr, "  path          WebSocket path (e.g., /api/v1/stream)\n");
    fprintf(stderr, "  bearer_token  Bearer token for Authorization header\n");
}

int main(int argc, char *argv[])
{
    char buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    if (argc != 3)
    {
        print_usage(argv[0]);
        return 1;
    }

    ws_path = argv[1];
    auth_token = argv[2];

    printf("WebSocket client test\n");
    printf("Host: %s:%s\n", WS_HOST, WS_PORT);
    printf("Path: %s\n", ws_path);
    printf("Auth: Bearer %s...\n", auth_token ? auth_token : "(none)");

    if (connect_to_server(WS_HOST, WS_PORT) < 0)
    {
        return 1;
    }

    if (perform_handshake() < 0)
    {
        close(sockfd);
        return 1;
    }

    websocket_parser_init(&parser);
    websocket_parser_settings_init(&settings);
    settings.on_frame_header = on_frame_header;
    settings.on_frame_body = on_frame_body;
    settings.on_frame_end = on_frame_end;

    printf("Entering receive loop (press Ctrl+C to exit)...\n");

    while (1)
    {
        fd_set readfds;
        struct timeval tv;
        int retval;

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = 5;
        tv.tv_usec = 0;

        retval = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (retval == -1)
        {
            if (errno == EINTR)
                continue;
            perror("select");
            break;
        }
        else if (retval == 0)
        {
            printf("Timeout: sending ping\n");
            if (send_text_message(PING_JSON) < 0)
            {
                fprintf(stderr, "Failed to send ping\n");
                break;
            }
            continue;
        }

        bytes_received = recv(sockfd, buffer, sizeof(buffer), 0);

        if (bytes_received < 0)
        {
            if (errno == EINTR)
                continue;
            perror("recv");
            break;
        }

        if (bytes_received == 0)
        {
            printf("Connection closed by server\n");
            break;
        }

        printf("Received %zd bytes\n", bytes_received);

        size_t parsed = websocket_parser_execute(&parser, &settings, buffer, bytes_received);

        if (parsed != (size_t)bytes_received)
        {
            fprintf(stderr, "WebSocket parser error: parsed %zu of %zd bytes\n",
                    parsed, bytes_received);
            break;
        }
    }

    send_close_frame();
    close(sockfd);

    return 0;
}