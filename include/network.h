#ifndef NETWORK_H
#define NETWORK_H

#include <stddef.h>

/* Incoming text commands are accumulated here until a full line arrives. */
#define NET_BUFFER_SIZE 4096

typedef struct {
    char data[NET_BUFFER_SIZE];
    size_t used;
} NetBuffer;

/* Socket setup and line-based IO helpers. */
int create_server(int port);
int accept_client(int server_fd);
int create_client(const char *host, int port);
int set_nonblocking(int fd);
int send_all(int fd, const char *data, size_t length);
int send_text(int fd, const char *text);
void net_buffer_init(NetBuffer *buffer);
int net_read_into_buffer(int fd, NetBuffer *buffer);
int net_next_line(NetBuffer *buffer, char *line, size_t line_size);

#endif
