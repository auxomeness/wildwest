#include "game.h"
#include "network.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
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
    /* Put the client terminal back in its normal state on exit. */
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
    /* Raw mode makes the remote client feel as immediate as the server side. */
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
    /* Nonblocking keyboard polling keeps rendering and networking smooth. */
    struct timeval timeout = {0, 0};
    fd_set input_set;

    FD_ZERO(&input_set);
    FD_SET(STDIN_FILENO, &input_set);

    return select(STDIN_FILENO + 1, &input_set, NULL, NULL, &timeout) > 0;
}

static int read_key(void)
{
    /* Translate terminal bytes into the same logical inputs the server uses. */
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

static void render_connecting_screen(const char *host, int port)
{
    /* Shown before the client receives the first real state packet. */
    printf("\033[2J\033[H");
    printf("Wild West Quick Draw | Player 2\n");
    printf("Connecting to %s:%d...\n", host, port);
    printf("Controls: Left/Right Arrow = move | [ = shoot | ] = heal | Space = lock | q = quit\n");
    fflush(stdout);
}

static int parse_state_line(GameState *game, const char *line)
{
    /* Convert the server's text packet back into a local GameState copy. */
    int phase;
    int phase_time_ms;
    int round_number;
    int winner;
    int running;
    int p1_hp;
    int p2_hp;
    int p1_col;
    int p2_col;
    int p1_potions;
    int p2_potions;
    int p1_action;
    int p2_action;
    int p1_locked;
    int p2_locked;
    int p1_ready;
    int p2_ready;
    int p1_result;
    int p2_result;
    int bullet1_row;
    int bullet1_col;
    int bullet1_active;
    int bullet2_row;
    int bullet2_col;
    int bullet2_active;

    if (sscanf(line,
               "STATE %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d",
               &phase,
               &phase_time_ms,
               &round_number,
               &winner,
               &running,
               &p1_hp,
               &p2_hp,
               &p1_col,
               &p2_col,
               &p1_potions,
               &p2_potions,
               &p1_action,
               &p2_action,
               &p1_locked,
               &p2_locked,
               &p1_ready,
               &p2_ready,
               &p1_result,
               &p2_result,
               &bullet1_row,
               &bullet1_col,
               &bullet1_active,
               &bullet2_row,
               &bullet2_col,
               &bullet2_active) != 25) {
        return 0;
    }

    game->phase = (Phase)phase;
    game->phase_time_ms = phase_time_ms;
    game->round_number = round_number;
    game->winner = winner;
    game->running = running;
    game->p1.hp = p1_hp;
    game->p2.hp = p2_hp;
    game->p1.col = p1_col;
    game->p2.col = p2_col;
    game->p1.potions = p1_potions;
    game->p2.potions = p2_potions;
    game->p1.action = (Action)p1_action;
    game->p2.action = (Action)p2_action;
    game->p1.locked = p1_locked;
    game->p2.locked = p2_locked;
    game->p1_ready = p1_ready;
    game->p2_ready = p2_ready;
    game->p1_result = (ResolveResult)p1_result;
    game->p2_result = (ResolveResult)p2_result;
    game->bullet1_row = bullet1_row;
    game->bullet1_col = bullet1_col;
    game->bullet1_active = bullet1_active;
    game->bullet2_row = bullet2_row;
    game->bullet2_col = bullet2_col;
    game->bullet2_active = bullet2_active;

    return 1;
}

int main(int argc, char **argv)
{
    /* Client loop: send local input, receive state snapshots, render them. */
    int socket_fd;
    int port = DEFAULT_PORT;
    int keep_running = 1;
    int have_state = 0;
    NetBuffer buffer;
    GameState game;
    char line[128];
    char render_key[256];
    char last_render_key[256];

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server-ip> [port]\n", argv[0]);
        return 1;
    }

    if (argc > 2) {
        port = atoi(argv[2]);
    }

    signal(SIGPIPE, SIG_IGN);
    render_connecting_screen(argv[1], port);
    last_render_key[0] = '\0';

    socket_fd = create_client(argv[1], port);
    if (socket_fd < 0) {
        return 1;
    }

    if (set_nonblocking(socket_fd) != 0) {
        close(socket_fd);
        return 1;
    }

    if (enable_raw_mode() != 0) {
        close(socket_fd);
        return 1;
    }

    memset(&game, 0, sizeof(game));
    net_buffer_init(&buffer);

    while (keep_running) {
        int read_status;

        while (key_available()) {
            int key = read_key();

            if (key == KEY_LEFT) {
                if (send_text(socket_fd, "LEFT\n") != 0) {
                    keep_running = 0;
                    break;
                }
            } else if (key == KEY_RIGHT) {
                if (send_text(socket_fd, "RIGHT\n") != 0) {
                    keep_running = 0;
                    break;
                }
            } else if (key == KEY_SELECT_SHOOT) {
                if (send_text(socket_fd, "SHOOT\n") != 0) {
                    keep_running = 0;
                    break;
                }
            } else if (key == KEY_SELECT_HEAL) {
                if (send_text(socket_fd, "HEAL\n") != 0) {
                    keep_running = 0;
                    break;
                }
            } else if (key == KEY_SPACE) {
                if (send_text(socket_fd, "LOCK\n") != 0) {
                    keep_running = 0;
                    break;
                }
            } else if (key == KEY_QUIT) {
                send_text(socket_fd, "QUIT\n");
                keep_running = 0;
                break;
            }
        }

        do {
            read_status = net_read_into_buffer(socket_fd, &buffer);

            if (read_status < 0) {
                keep_running = 0;
                break;
            }
        } while (read_status > 0);

        while (net_next_line(&buffer, line, sizeof(line))) {
            if (parse_state_line(&game, line)) {
                have_state = 1;
            }
        }

        if (have_state) {
            game_build_display_key(&game, 2, render_key, sizeof(render_key));
            if (strcmp(render_key, last_render_key) != 0) {
                game_render(&game, 2);
                strcpy(last_render_key, render_key);
            }

            if (!game.running) {
                keep_running = 0;
            }
        }

        if (keep_running) {
            usleep(50000);
        }
    }

    close(socket_fd);
    return 0;
}
