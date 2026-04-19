#include "./include/network.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int create_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 2);

    return server_fd;
}

int accept_client(int server_fd) {
    return accept(server_fd, NULL, NULL);
}

int send_msg(int client_fd, const char* msg) {
    return send(client_fd, msg, strlen(msg), 0);
}

int recv_msg(int client_fd, char* buffer) {
    return recv(client_fd, buffer, 1024, 0);
}
