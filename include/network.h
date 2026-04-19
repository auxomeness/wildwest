#ifndef NETWORK_H
#define NETWORK_H

int create_server(int port);
int accept_client(int server_fd);
int send_msg(int client_fd, const char* msg);
int recv_msg(int client_fd, char* buffer);

#endif
