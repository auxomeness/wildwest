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
    printf("Controls: Arrows = move/select | Space = lock/shoot | q = quit\n");
    fflush(stdout);
}

static int next_state_int(const char **cursor, int *value)
{
    char *end;
    long parsed;

    while (**cursor == ' ') {
        (*cursor)++;
    }

    if (**cursor == '\0') {
        return 0;
    }

    parsed = strtol(*cursor, &end, 10);
    if (end == *cursor) {
        return 0;
    }

    *value = (int)parsed;
    *cursor = end;
    return 1;
}

static int parse_state_line(GameState *game, const char *line)
{
    /* Convert the server's text packet back into a local GameState copy. */
    const char *cursor;
    int value;
    int index;

    if (strncmp(line, "STATE", 5) != 0) {
        return 0;
    }

    cursor = line + 5;

#define READ_STATE_INT(target)       \
    do {                             \
        if (!next_state_int(&cursor, &value)) { \
            return 0;                \
        }                            \
        (target) = value;            \
    } while (0)

    READ_STATE_INT(value);
    game->phase = (Phase)value;
    READ_STATE_INT(game->phase_time_ms);
    READ_STATE_INT(game->round_number);
    READ_STATE_INT(game->winner);
    READ_STATE_INT(game->running);
    READ_STATE_INT(game->p1.hp);
    READ_STATE_INT(game->p2.hp);
    READ_STATE_INT(game->p1.col);
    READ_STATE_INT(game->p2.col);
    READ_STATE_INT(game->p1.potions);
    READ_STATE_INT(game->p2.potions);
    READ_STATE_INT(value);
    game->p1.action = (Action)value;
    READ_STATE_INT(value);
    game->p2.action = (Action)value;
    READ_STATE_INT(game->p1.locked);
    READ_STATE_INT(game->p2.locked);
    READ_STATE_INT(game->p1_ready);
    READ_STATE_INT(game->p2_ready);
    READ_STATE_INT(value);
    game->p1_result = (ResolveResult)value;
    READ_STATE_INT(value);
    game->p2_result = (ResolveResult)value;
    READ_STATE_INT(game->bullet1_row);
    READ_STATE_INT(game->bullet1_col);
    READ_STATE_INT(game->bullet1_active);
    READ_STATE_INT(game->bullet2_row);
    READ_STATE_INT(game->bullet2_col);
    READ_STATE_INT(game->bullet2_active);
    READ_STATE_INT(game->p1_sudden_death_vote);
    READ_STATE_INT(game->p2_sudden_death_vote);
    READ_STATE_INT(game->p1_sudden_death_vote_locked);
    READ_STATE_INT(game->p2_sudden_death_vote_locked);
    READ_STATE_INT(game->sudden_death_declined);
    READ_STATE_INT(game->p1_ammo);
    READ_STATE_INT(game->p2_ammo);
    READ_STATE_INT(game->p1_reload_ms);
    READ_STATE_INT(game->p2_reload_ms);
    READ_STATE_INT(game->p1_bullet_step_ms);
    READ_STATE_INT(game->p2_bullet_step_ms);
    READ_STATE_INT(game->p1_damage_feedback);
    READ_STATE_INT(game->p2_damage_feedback);
    READ_STATE_INT(game->p1_damage_feedback_ms);
    READ_STATE_INT(game->p2_damage_feedback_ms);

    for (index = 0; index < SUDDEN_DEATH_MAX_AMMO; index++) {
        READ_STATE_INT(game->p1_sd_bullet_row[index]);
        READ_STATE_INT(game->p1_sd_bullet_col[index]);
        READ_STATE_INT(game->p1_sd_bullet_active[index]);
        READ_STATE_INT(game->p1_sd_bullet_step_ms[index]);
        READ_STATE_INT(game->p2_sd_bullet_row[index]);
        READ_STATE_INT(game->p2_sd_bullet_col[index]);
        READ_STATE_INT(game->p2_sd_bullet_active[index]);
        READ_STATE_INT(game->p2_sd_bullet_step_ms[index]);
    }

#undef READ_STATE_INT

    return 1;
}

int main(int argc, char **argv)
{
    /* Client loop: send local input, receive state snapshots, render them. */
    int socket_fd;
    int port = DEFAULT_PORT;
    int keep_running = 1;
    int quit_armed = 0;
    int have_state = 0;
    NetBuffer buffer;
    GameState game;
    char line[NET_BUFFER_SIZE];
    char render_key[1024];
    char last_render_key[1024];

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

            if (key == KEY_QUIT) {
                if (!quit_armed) {
                    quit_armed = 1;
                    continue;
                }

                quit_armed = 0;

                if (have_state && (game.phase == PHASE_WAITING || game.phase == PHASE_GAME_OVER)) {
                    send_text(socket_fd, "QUIT\n");
                    keep_running = 0;
                    break;
                }

                if (send_text(socket_fd, "QUIT\n") != 0) {
                    keep_running = 0;
                    break;
                }

                continue;
            }

            quit_armed = 0;

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
            snprintf(render_key + strlen(render_key),
                     sizeof(render_key) - strlen(render_key),
                     "|%d",
                     quit_armed);
            if (strcmp(render_key, last_render_key) != 0) {
                game_render(&game, 2, quit_armed);
                strcpy(last_render_key, render_key);
            }
        }

        if (keep_running) {
            usleep(50000);
        }
    }

    close(socket_fd);
    return 0;
}
