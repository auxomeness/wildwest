#include "./include/server.h"
#include "./include/network.h"
#include "./include/game.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    int p1_fd;
    int p2_fd;
} Session;

Action parse_action(char* msg) {
    if (strncmp(msg, "\e[{n}A", 7) == 0) return MOVE_UP;
    if (strncmp(msg, "\e[{n}B", 7) == 0) return MOVE_DOWN;
    if (strncmp(msg, "SHOOT", 5) == 0) return SHOOT;
    if (strncmp(msg, "HEAL", 4) == 0) return HEAL;
    return SHOOT;
}

void* game_thread(void* arg) {
    Session* s = (Session*)arg;
    Game* g = game_create();

    char buf1[1024], buf2[1024];

    while (!game_is_over(g)) {
        recv_msg(s->p1_fd, buf1);
        recv_msg(s->p2_fd, buf2);

        Action a1 = parse_action(buf1);
        Action a2 = parse_action(buf2);

        game_resolve_turn(g, a1, a2);

        send_msg(s->p1_fd, "TURN_RESOLVED\n");
        send_msg(s->p2_fd, "TURN_RESOLVED\n");
    }

    close(s->p1_fd);
    close(s->p2_fd);
    return NULL;
}

void start_server(int port) {
    int server_fd = create_server(port);

    while (1) {
        printf("Waiting for players...\n");

        int p1 = accept_client(server_fd);
        int p2 = accept_client(server_fd);

        Session* s = malloc(sizeof(Session));
        s->p1_fd = p1;
        s->p2_fd = p2;

        pthread_t tid;
        pthread_create(&tid, NULL, game_thread, s);
        pthread_detach(tid);
    }
}
