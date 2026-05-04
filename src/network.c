#include "network.h"
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int create_server(int port)
{
    /* Create a TCP listening socket for the server process. */
    int server_fd;
    int opt = 1;
    struct sockaddr_in addr;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 1) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int accept_client(int server_fd)
{
    /* Block until one client connects. */
    return accept(server_fd, NULL, NULL);
}

int create_client(const char *host, int port)
{
    /* Connect the local client process to the remote server. */
    int client_fd;
    struct sockaddr_in addr;

    client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host, &addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server IP: %s\n", host);
        close(client_fd);
        return -1;
    }

    if (connect(client_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("connect");
        close(client_fd);
        return -1;
    }

    return client_fd;
}

int set_nonblocking(int fd)
{
    /* Nonblocking sockets keep the terminal loop responsive. */
    int flags = fcntl(fd, F_GETFL, 0);

    if (flags < 0) {
        perror("fcntl");
        return -1;
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        perror("fcntl");
        return -1;
    }

    return 0;
}

int send_all(int fd, const char *data, size_t length)
{
    /* Keep sending until the whole buffer has been written. */
    size_t total = 0;

    while (total < length) {
        ssize_t sent = send(fd, data + total, length - total, 0);

        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }

            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(1000);
                continue;
            }

            return -1;
        }

        total += (size_t)sent;
    }

    return 0;
}

int send_text(int fd, const char *text)
{
    return send_all(fd, text, strlen(text));
}

void net_buffer_init(NetBuffer *buffer)
{
    buffer->used = 0;
}

int net_read_into_buffer(int fd, NetBuffer *buffer)
{
    /* Pull raw bytes into the line buffer without blocking the game loop. */
    ssize_t received;
    size_t space_left;

    if (buffer->used >= NET_BUFFER_SIZE - 1) {
        return -1;
    }

    space_left = (NET_BUFFER_SIZE - 1) - buffer->used;
    received = recv(fd, buffer->data + buffer->used, space_left, 0);

    if (received == 0) {
        return -1;
    }

    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }

        if (errno == EINTR) {
            return 0;
        }

        return -1;
    }

    buffer->used += (size_t)received;
    buffer->data[buffer->used] = '\0';

    return 1;
}

int net_next_line(NetBuffer *buffer, char *line, size_t line_size)
{
    /* Extract exactly one newline-terminated command at a time. */
    size_t index;

    for (index = 0; index < buffer->used; index++) {
        if (buffer->data[index] == '\n') {
            size_t copy_len = index;
            size_t remaining = buffer->used - (index + 1);

            if (copy_len > 0 && buffer->data[copy_len - 1] == '\r') {
                copy_len -= 1;
            }

            if (copy_len >= line_size) {
                copy_len = line_size - 1;
            }

            memcpy(line, buffer->data, copy_len);
            line[copy_len] = '\0';

            memmove(buffer->data, buffer->data + index + 1, remaining);
            buffer->used = remaining;
            buffer->data[buffer->used] = '\0';

            return 1;
        }
    }

    return 0;
}
