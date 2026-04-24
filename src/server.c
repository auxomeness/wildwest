#include "server.h"
#include "game.h"
#include "network.h"
#include "player.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

enum {
    KEY_NONE = 0,
    KEY_LEFT = 1,
    KEY_RIGHT = 2,
    KEY_SELECT_SHOOT = 3,
    KEY_SELECT_HEAL = 4,
    KEY_SPACE = 5,
    KEY_QUIT = 6
};

static struct termios original_termios;
static int raw_mode_enabled = 0;
static int ui_mode_enabled = 0;

static void disable_ui_mode(void)
{
    /* Restore the normal terminal screen when the game exits. */
    if (ui_mode_enabled) {
        printf("\033[?25h\033[?1049l");
        fflush(stdout);
        ui_mode_enabled = 0;
    }
}

static void restore_terminal(void)
{
    if (raw_mode_enabled) {
        tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
        raw_mode_enabled = 0;
    }

    disable_ui_mode();
}

static int enable_raw_mode(void)
{
    /* Raw mode lets the server react to single key presses immediately. */
    struct termios raw;

    if (tcgetattr(STDIN_FILENO, &original_termios) != 0) {
        perror("tcgetattr");
        return -1;
    }

    raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) != 0) {
        perror("tcsetattr");
        return -1;
    }

    raw_mode_enabled = 1;
    atexit(restore_terminal);
    printf("\033[?1049h\033[?25l");
    fflush(stdout);
    ui_mode_enabled = 1;

    return 0;
}

static int key_available(void)
{
    /* Nonblocking keyboard poll for the local server player. */
    struct timeval timeout = {0, 0};
    fd_set input_set;

    FD_ZERO(&input_set);
    FD_SET(STDIN_FILENO, &input_set);

    return select(STDIN_FILENO + 1, &input_set, NULL, NULL, &timeout) > 0;
}

static int read_key(void)
{
    /* Translate raw keyboard bytes into game input tokens. */
    char c;

    if (read(STDIN_FILENO, &c, 1) <= 0) {
        return KEY_NONE;
    }

    if (c == ' ') {
        return KEY_SPACE;
    }

    if (c == 'q' || c == 'Q') {
        return KEY_QUIT;
    }

    if (c == '[') {
        return KEY_SELECT_SHOOT;
    }

    if (c == ']') {
        return KEY_SELECT_HEAL;
    }

    if (c == '\033') {
        char sequence[2];

        if (read(STDIN_FILENO, &sequence[0], 1) <= 0) {
            return KEY_NONE;
        }

        if (read(STDIN_FILENO, &sequence[1], 1) <= 0) {
            return KEY_NONE;
        }

        if (sequence[0] == '[' && sequence[1] == 'D') {
            return KEY_LEFT;
        }

        if (sequence[0] == '[' && sequence[1] == 'C') {
            return KEY_RIGHT;
        }

    }

    return KEY_NONE;
}

static int elapsed_ms(struct timespec start, struct timespec end)
{
    /* Timing is phase-based, so every loop computes a delta in milliseconds. */
    long seconds = end.tv_sec - start.tv_sec;
    long nanoseconds = end.tv_nsec - start.tv_nsec;

    return (int)(seconds * 1000L + nanoseconds / 1000000L);
}

static int send_state(int client_fd, const GameState *game)
{
    /* Server sends one line of serialized state to the client every tick. */
    char state_line[320];
    int length;

    length = snprintf(state_line,
                      sizeof(state_line),
                      "STATE %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
                      game->phase,
                      game->phase_time_ms,
                      game->round_number,
                      game->winner,
                      game->running,
                      game->p1.hp,
                      game->p2.hp,
                      game->p1.col,
                      game->p2.col,
                      game->p1.potions,
                      game->p2.potions,
                      game->p1.action,
                      game->p2.action,
                      game->p1.locked,
                      game->p2.locked,
                      game->p1_ready,
                      game->p2_ready,
                      game->p1_result,
                      game->p2_result,
                      game->bullet1_row,
                      game->bullet1_col,
                      game->bullet1_active,
                      game->bullet2_row,
                      game->bullet2_col,
                      game->bullet2_active);

    if (length <= 0) {
        return -1;
    }

    return send_all(client_fd, state_line, (size_t)length);
}

static void handle_local_input(GameState *game, int key)
{
    /* Player 1 input is applied directly to the authoritative state. */
    if (key == KEY_QUIT) {
        game->winner = 2;
        game->phase = PHASE_GAME_OVER;
        game->running = 0;
        return;
    }

    if (game->phase == PHASE_WAITING) {
        if (key == KEY_SPACE) {
            game->p1_ready = 1;
        }
    } else if (game->phase == PHASE_MOVE && !game->p1.locked) {
        if (key == KEY_LEFT) {
            player_move(&game->p1, -1, 0, GRID_WIDTH - 1);
        } else if (key == KEY_RIGHT) {
            player_move(&game->p1, 1, 0, GRID_WIDTH - 1);
        } else if (key == KEY_SPACE) {
            player_lock(&game->p1);
        }
    } else if (game->phase == PHASE_ACTION && !game->p1.locked) {
        if (key == KEY_SELECT_SHOOT) {
            player_set_action(&game->p1, ACTION_SHOOT);
        } else if (key == KEY_SELECT_HEAL) {
            player_set_action(&game->p1, ACTION_HEAL);
        } else if (key == KEY_SPACE) {
            player_lock(&game->p1);
        }
    }
}

static void handle_remote_command(GameState *game, const char *line)
{
    /* Player 2 input arrives as text commands over the socket. */
    if (strcmp(line, "QUIT") == 0) {
        game->winner = 1;
        game->phase = PHASE_GAME_OVER;
        game->running = 0;
        return;
    }

    if (game->phase == PHASE_WAITING) {
        if (strcmp(line, "LOCK") == 0) {
            game->p2_ready = 1;
        }
    } else if (game->phase == PHASE_MOVE && !game->p2.locked) {
        if (strcmp(line, "LEFT") == 0) {
            player_move(&game->p2, -1, 0, GRID_WIDTH - 1);
        } else if (strcmp(line, "RIGHT") == 0) {
            player_move(&game->p2, 1, 0, GRID_WIDTH - 1);
        } else if (strcmp(line, "LOCK") == 0) {
            player_lock(&game->p2);
        }
    } else if (game->phase == PHASE_ACTION && !game->p2.locked) {
        if (strcmp(line, "SHOOT") == 0) {
            player_set_action(&game->p2, ACTION_SHOOT);
        } else if (strcmp(line, "HEAL") == 0) {
            player_set_action(&game->p2, ACTION_HEAL);
        } else if (strcmp(line, "LOCK") == 0) {
            player_lock(&game->p2);
        }
    }
}

static void update_match(GameState *game)
{
    /* Phase switching is owned by the server and driven by timers or locks. */
    if (game->phase == PHASE_WAITING) {
        if (game->p1_ready && game->p2_ready) {
            game_start_move_phase(game);
        }
        return;
    }

    if (game->phase == PHASE_MOVE) {
        if ((game->p1.locked && game->p2.locked) || game->phase_time_ms >= MOVE_PHASE_MS) {
            game_start_action_phase(game);
        }
        return;
    }

    if (game->phase == PHASE_ACTION) {
        if ((game->p1.locked && game->p2.locked) || game->phase_time_ms >= ACTION_PHASE_MS) {
            game_start_resolve_phase(game);
        }
        return;
    }

    if (game->phase == PHASE_RESOLVE) {
        game_update_bullets(game);

        if (game->phase_time_ms >= RESOLVE_PHASE_MS) {
            if (game->winner != 0) {
                game->phase = PHASE_GAME_OVER;
                game->running = 0;
            } else {
                game_start_move_phase(game);
            }
        }
    }
}

int start_server(int port)
{
    /* Main server loop: accept one client, run the match, mirror state out. */
    int server_fd;
    int client_fd;
    int keep_running = 1;
    int client_alive = 1;
    GameState game;
    NetBuffer input_buffer;
    struct timespec last_tick;
    char line[128];
    char render_key[256];
    char last_render_key[256];

    signal(SIGPIPE, SIG_IGN);
    srand((unsigned int)time(NULL));
    last_render_key[0] = '\0';

    server_fd = create_server(port);
    if (server_fd < 0) {
        return 1;
    }

    printf("Waiting for client on port %d...\n", port);
    fflush(stdout);

    client_fd = accept_client(server_fd);
    if (client_fd < 0) {
        perror("accept");
        close(server_fd);
        return 1;
    }

    if (set_nonblocking(client_fd) != 0) {
        close(client_fd);
        close(server_fd);
        return 1;
    }

    if (enable_raw_mode() != 0) {
        close(client_fd);
        close(server_fd);
        return 1;
    }

    net_buffer_init(&input_buffer);
    game_init(&game);
    clock_gettime(CLOCK_MONOTONIC, &last_tick);

    while (keep_running) {
        struct timespec now;
        int delta_ms;
        int read_status;

        clock_gettime(CLOCK_MONOTONIC, &now);
        delta_ms = elapsed_ms(last_tick, now);
        last_tick = now;

        if (delta_ms < 0) {
            delta_ms = 0;
        }

        if (delta_ms > 200) {
            delta_ms = 200;
        }

        game.phase_time_ms += delta_ms;

        while (key_available()) {
            handle_local_input(&game, read_key());
        }

        do {
            read_status = net_read_into_buffer(client_fd, &input_buffer);

            if (read_status < 0) {
                client_alive = 0;
                game.winner = 1;
                game.phase = PHASE_GAME_OVER;
                game.running = 0;
                break;
            }
        } while (read_status > 0);

        while (client_alive && net_next_line(&input_buffer, line, sizeof(line))) {
            handle_remote_command(&game, line);
        }

        if (game.phase != PHASE_GAME_OVER) {
            update_match(&game);
        }

        if (client_alive && send_state(client_fd, &game) != 0) {
            client_alive = 0;
            game.winner = 1;
            game.phase = PHASE_GAME_OVER;
            game.running = 0;
        }

        game_build_display_key(&game, 1, render_key, sizeof(render_key));
        if (strcmp(render_key, last_render_key) != 0) {
            game_render(&game, 1);
            strcpy(last_render_key, render_key);
        }

        if (!game.running) {
            keep_running = 0;
        } else {
            usleep(50000);
        }
    }

    close(client_fd);
    close(server_fd);

    return 0;
}
