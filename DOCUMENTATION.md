# Wild West Quick Draw Documentation

## Overview

Wild West Quick Draw is a two-player terminal duel written in C.

The game runs as two programs:

- `server`: Player 1 and the authoritative game loop.
- `client`: Player 2, connected to the server through TCP.

The server owns the real state. It controls timers, phase changes, shots, healing, crit chance, sudden death, winner checks, and the state packet sent to the client. The client sends input commands and renders the latest state packet it receives.

This design keeps the game from desyncing. There is one source of truth: the server.

## Folder Structure

```text
wildwest/
├── include/
│   ├── game.h
│   ├── network.h
│   ├── player.h
│   └── server.h
├── src/
│   ├── client.c
│   ├── game.c
│   ├── main.c
│   ├── network.c
│   ├── player.c
│   └── server.c
├── makefile
├── README.md
├── SETUP.md
└── DOCUMENTATION.md
```

## Build Output

The makefile builds two binaries:

- `server`
- `client`

The compiler uses `-Iinclude`, so source files can write:

```c
#include "game.h"
```

instead of hardcoding the folder path.

## Match Phases

### `WAITING`

This is the start screen.

- Both players see the `WILD WEST` title.
- Both players press `Space` to get ready.
- The battlefield is not drawn yet.
- Pressing `q` twice exits directly from this screen.

### `MOVE`

Both players choose a lane.

- Timer: `15` seconds.
- Enemy position is hidden.
- `Left Arrow` moves left.
- `Right Arrow` moves right.
- `Space` locks the position.

### `ACTION`

Both players choose what to do.

- Timer: `10` seconds.
- Enemy action is hidden.
- `Left Arrow` selects `SHOOT`.
- `Right Arrow` selects `HEAL`.
- `Space` locks the action.

### `RESOLVE`

The round resolves.

- Enemy position is revealed.
- Shooting and healing are applied.
- Bullet animation is shown.
- Damage feedback appears beside the damaged player.

### `SUDDEN_DEATH_OFFER`

This phase starts only if both players are at `20` HP or below and nobody has won yet.

- `Left Arrow` votes `YES`.
- `Right Arrow` votes `NO`.
- `Space` locks the vote.
- Both players must vote `YES` to enter sudden death.
- If either player votes `NO`, the normal game continues.

### `SUDDEN_DEATH_READY`

Both players prepare for sudden death.

- HP resets to `100`.
- Ammo resets to `5`.
- Both players press `Space`.

### `SUDDEN_DEATH_BATTLE`

Sudden death is live.

- Both players are visible.
- Arena height is the same as the normal game arena.
- `Left Arrow` and `Right Arrow` move.
- `Space` shoots.
- Ammo starts at `5`.
- Ammo reloads to `5` after `2` seconds when empty.
- Bullet damage is `10`.
- First player to kill the opponent wins.

### `GAME_OVER`

The result banner is displayed.

- Winner sees `WINNER`.
- Loser sees `LOSER`.
- Draw shows `DRAW`.
- Press `q` twice to exit.

## Current Controls

Start screen:

- `Space` = ready
- `q` twice = quit directly

Move phase:

- `Left Arrow` = move left
- `Right Arrow` = move right
- `Space` = lock position

Action phase:

- `Left Arrow` = select `SHOOT`
- `Right Arrow` = select `HEAL`
- `Space` = lock action

Sudden death vote:

- `Left Arrow` = vote `YES`
- `Right Arrow` = vote `NO`
- `Space` = lock vote

Sudden death battle:

- `Left Arrow` = move left
- `Right Arrow` = move right
- `Space` = shoot

General:

- `q` twice = quit

## Combat Rules

Normal combat:

- Shot hit deals `20` damage.
- Critical hit deals `30` damage.
- Miss backfires and deals `10` self-damage.
- Miss backfire can kill the shooter.
- Heal restores `30` HP.
- Heal uses `1` potion.
- Each player starts with `3` potions.

Critical chance:

- Each player starts with `12%` crit chance.
- If a shot hits but does not crit, crit chance increases.
- If a shot crits, crit chance resets to the default.

Sudden death:

- HP resets to `100`.
- Ammo starts at `5`.
- Bullet damage is `10`.
- Ammo reloads after `2` seconds when empty.
- Multiple bullets can exist at the same time.

## Header Files

### `include/player.h`

This file defines player-level data.

`Action`

- `ACTION_NONE`
- `ACTION_SHOOT`
- `ACTION_HEAL`

`ResolveResult`

- `RESULT_NONE`
- `RESULT_SHOT_HIT`
- `RESULT_SHOT_CRIT`
- `RESULT_SHOT_MISS`
- `RESULT_HEAL`
- `RESULT_HEAL_FAIL`

`Player`

- `col`: current lane.
- `hp`: current health.
- `potions`: remaining heals.
- `crit_chance`: current critical-hit chance.
- `action`: selected action.
- `locked`: whether the player already committed in the current phase.

### `include/game.h`

This file defines shared constants, phases, and the full replicated game state.

Important constants:

- `DEFAULT_PORT`: default TCP port.
- `GRID_HEIGHT`: arena height.
- `GRID_WIDTH`: number of lanes.
- `MAX_HP`: health cap.
- `MAX_POTIONS`: starting potion count.
- `SHOT_DAMAGE`: normal hit damage.
- `CRIT_DAMAGE`: critical hit damage.
- `MISS_DAMAGE`: self-damage on miss.
- `HEAL_AMOUNT`: healing amount.
- `SUDDEN_DEATH_THRESHOLD`: HP threshold for sudden death offer.
- `SUDDEN_DEATH_MAX_AMMO`: sudden death ammo cap.
- `SUDDEN_DEATH_RELOAD_MS`: reload time.
- `SUDDEN_DEATH_DAMAGE`: sudden death bullet damage.

`GameState`

- `p1`, `p2`: the two players.
- `p1_ready`, `p2_ready`: ready flags.
- `p1_result`, `p2_result`: last resolve results.
- `bullet1_row`, `bullet1_col`, `bullet1_active`: normal resolve bullet for Player 1.
- `bullet2_row`, `bullet2_col`, `bullet2_active`: normal resolve bullet for Player 2.
- `p1_sudden_death_vote`, `p2_sudden_death_vote`: sudden death choices.
- `p1_sudden_death_vote_locked`, `p2_sudden_death_vote_locked`: vote locks.
- `sudden_death_declined`: prevents asking again after a `NO`.
- `p1_ammo`, `p2_ammo`: sudden death ammo.
- `p1_reload_ms`, `p2_reload_ms`: reload timers.
- `p1_damage_feedback`, `p2_damage_feedback`: damage text shown beside players.
- `p1_sd_bullet_*`, `p2_sd_bullet_*`: sudden death bullet pools.
- `phase`: current phase.
- `phase_time_ms`: time spent in the current phase.
- `round_number`: current round.
- `winner`: `0` none, `1` Player 1, `2` Player 2, `3` draw.
- `running`: match running flag.

### `include/network.h`

This file defines socket helpers and the line buffer.

`NetBuffer`

- `data`: incoming bytes.
- `used`: number of bytes stored.

The state packet is text-based, so the buffer waits until a full line arrives.

### `include/server.h`

This file exposes:

- `start_server`

## Source Files

### `src/main.c`

Entry point for the server binary.

It reads an optional port argument and calls:

```c
start_server(port);
```

### `src/player.c`

This file handles player-specific rules.

`player_init`

- Sets start column.
- Sets HP to `100`.
- Sets potions to `3`.
- Sets default crit chance.
- Clears action and lock state.

`player_move`

- Moves left or right.
- Clamps the player inside the arena.

`player_lock` and `player_unlock`

- Commit or clear phase lock state.

`player_clear_action`

- Resets action to `ACTION_NONE`.

`player_set_action`

- Sets action to `SHOOT` or `HEAL`.

`player_apply_shot`

- Checks if the shooter chose `SHOOT`.
- If columns match, applies hit or crit damage.
- If columns do not match, applies miss backfire damage.

`player_apply_heal`

- Checks if healing is allowed.
- Restores HP.
- Spends one potion.

### `src/game.c`

This file owns game state transitions and terminal rendering.

Important internal helpers:

- `clear_bullets`: clears normal bullet animation.
- `clear_sudden_death_bullets`: clears sudden death bullet pools.
- `clear_damage_feedback`: removes damage popup text.
- `add_damage_feedback`: stores damage text for a player.
- `update_winner`: calculates winner or draw.
- `local_player`: returns the player for the current terminal perspective.
- `enemy_player`: returns the opponent for the current terminal perspective.

Phase functions:

- `game_init`
- `game_start_move_phase`
- `game_start_action_phase`
- `game_start_resolve_phase`
- `game_start_sudden_death_offer`
- `game_start_sudden_death_ready`
- `game_start_sudden_death_battle`

Rendering functions:

- `game_build_display_key`: builds a compact visible-state key.
- `game_render`: draws the current frame.

The renderer:

- Uses the alternate terminal screen.
- Draws a figlet-style start banner.
- Draws boxed headers and battlefield.
- Hides enemy position during `MOVE` and `ACTION`.
- Hides enemy action during `ACTION`.
- Shows HP bars, HP percentage, potions, or ammo.
- Shows damage feedback beside the player when damage is applied.

#### Banner text and figlet

The large title, sudden death, winner, loser, and draw banners were made with `figlet` during development.

The important part is that `figlet` is not used at runtime.

The workflow was:

1. Generate banner text with `figlet`.
2. Review which font style looked readable in the terminal.
3. Copy the generated ASCII output into C string arrays.
4. Print those arrays from `game.c`.

Because the generated text is already stored in the source code, a new machine does not need to install `figlet` to build or run the game. `figlet` was only a development helper, not a project dependency.

### `src/network.c`

This file handles TCP setup and line-based transport.

`create_server`

- Creates a socket.
- Enables address reuse.
- Binds to the port.
- Starts listening.

`accept_client`

- Waits for one client.

`create_client`

- Converts the server IP string.
- Connects to the server.

`set_nonblocking`

- Makes sockets nonblocking with `fcntl`.

`send_all`

- Sends until the whole buffer has been written.

`send_text`

- Sends a null-terminated string.

`net_read_into_buffer`

- Reads socket bytes into `NetBuffer`.

`net_next_line`

- Extracts one complete line from `NetBuffer`.

### `src/server.c`

This is the authoritative runtime.

Terminal functions:

- `enable_raw_mode`
- `disable_raw_mode`
- `enable_ui_mode`
- `disable_ui_mode`

These keep terminal input immediate and prevent the game from filling normal scrollback.

Input helpers:

- `key_available`
- `read_key`

State/network helpers:

- `send_state`: serializes `GameState`.
- `fire_sudden_death_shot`: creates a sudden death bullet if ammo is available.

Main game handlers:

- `handle_local_input`: applies Player 1 keyboard input.
- `handle_remote_command`: applies Player 2 network commands.
- `update_match`: advances phases and timers.
- `start_server`: owns the main loop.

### `src/client.c`

This is Player 2's runtime.

It does not resolve game rules. It only:

- reads Player 2 keyboard input
- sends commands to the server
- receives state packets
- parses state packets
- renders the latest state

Important functions:

- `render_connecting_screen`
- `parse_state_line`
- `main`

## How The Code Works Internally

This section explains the logic behind the code, not just what each file contains.

### Why the project is split this way

The code is separated by responsibility.

`player.c` only knows player rules.

- It does not know sockets.
- It does not know terminal rendering.
- It does not know match phases.
- It only knows how a player moves, shoots, heals, locks, and stores stats.

`game.c` knows match rules and drawing.

- It decides what a phase means.
- It creates the next phase.
- It applies combat results.
- It renders the same state differently depending on player perspective.

`network.c` only knows socket transport.

- It creates sockets.
- It sends full text buffers.
- It receives raw bytes.
- It extracts complete lines.

`server.c` connects everything together.

- It reads Player 1 input.
- It receives Player 2 input.
- It updates the authoritative `GameState`.
- It sends that state to the client.

`client.c` is deliberately weaker than the server.

- It does not decide damage.
- It does not decide winners.
- It does not advance phases.
- It only sends commands and displays the server state.

This is intentional. A multiplayer game needs one authority. Here, that authority is the server.

### How one frame works

Each server loop does the same sequence:

1. Measure elapsed time since the last loop.
2. Read Player 1 keyboard input.
3. Read Player 2 commands from the socket.
4. Apply input to the current phase.
5. Update timers and phase transitions.
6. Send the full `GameState` to the client.
7. Render Player 1's view if the visible state changed.
8. Sleep briefly to avoid using the CPU constantly.

The client loop is simpler:

1. Read Player 2 keyboard input.
2. Send input as text commands to the server.
3. Read state packets from the server.
4. Parse the latest `STATE` packet.
5. Render Player 2's view if the visible state changed.

The client never runs `update_match`. That is the server's job.

### How input becomes gameplay

Keyboard input is converted into a small set of internal keys:

- left
- right
- space
- quit

The same key can mean different things depending on phase.

For example, `Left Arrow` means:

- move left during `MOVE`
- choose `SHOOT` during `ACTION`
- vote `YES` during `SUDDEN_DEATH_OFFER`
- move left during `SUDDEN_DEATH_BATTLE`

This is why input handling checks the current phase before applying the command. The command alone is not enough. The phase gives it meaning.

### How the phase system is built

The game is a state machine. The `phase` field decides what rules are active.

The server changes phases through functions such as:

- `game_start_move_phase`
- `game_start_action_phase`
- `game_start_resolve_phase`
- `game_start_sudden_death_offer`
- `game_start_sudden_death_ready`
- `game_start_sudden_death_battle`

Each phase-start function resets only the state that belongs to that phase.

For example, `game_start_move_phase` clears old actions, unlocks both players, clears old bullets, and increments the round. It does not reset HP because HP must carry across rounds.

`game_start_sudden_death_ready` does reset HP because sudden death is a separate duel mode where both players restart at `100` HP.

### How timers work

The server records how many milliseconds have passed in the current phase using `phase_time_ms`.

Each loop adds the frame delta to `phase_time_ms`.

The phase changes when either:

- both players lock early
- the timer reaches the phase limit
- a sudden-death vote is completed
- both players are ready
- a winner exists

Move and action phases have timers. Waiting and game-over do not need countdown timers.

### How hiding information works

The full `GameState` contains both players' positions and actions. The server sends it to the client because the renderer needs one shared structure.

Hidden information is handled inside `game_render`.

During `MOVE` and `ACTION`:

- enemy position is not drawn
- enemy action is shown as hidden during `ACTION`

During `RESOLVE`, `SUDDEN_DEATH_BATTLE`, and `GAME_OVER`:

- enemy position is visible

This means the data exists, but the display chooses not to show it until the correct phase.

### How normal shooting works

Normal shooting is resolved in `player_apply_shot`.

The logic is:

1. If the player did not choose `SHOOT`, nothing happens.
2. If both players are in the same lane, the shot hits.
3. If the shot hits, the game rolls crit chance.
4. A crit deals `30` damage and resets crit chance.
5. A non-crit hit deals `20` damage and increases crit chance.
6. If the players are not aligned, the shot misses.
7. A miss deals `10` self-damage to the shooter.

Miss damage can kill the shooter. This is why a player can lose by backfire.

### How healing works

Healing is resolved in `player_apply_heal`.

Healing fails if:

- the player is dead
- the player has no potions
- the player is already at full HP

If healing succeeds:

- HP increases by `30`
- HP is capped at `100`
- potion count decreases by `1`

Healing does not create a bullet animation.

### How resolve order works

In `game_start_resolve_phase`, both players' locked choices are applied for the round.

Shots are evaluated using the locked positions. Healing is then applied if selected.

After health changes, `update_winner` checks the result:

- Player 1 wins if Player 2 reaches `0`
- Player 2 wins if Player 1 reaches `0`
- draw if both reach `0`
- no winner if both are still alive

Damage feedback is calculated by comparing HP before and after the resolve.

### How sudden death offer works

After a normal resolve, the server checks:

- no winner exists
- sudden death was not already declined
- Player 1 HP is `20` or below
- Player 2 HP is `20` or below

If all are true, the game enters `SUDDEN_DEATH_OFFER`.

Both players vote. The rule is strict:

- both `YES` means sudden death starts
- any `NO` means normal play continues

### How sudden death battle works

Sudden death changes the game from turn-based resolve into live bullet movement.

When sudden death starts:

- both HP values reset to `100`
- both columns reset to the center
- both ammo counts reset to `5`
- old bullets and feedback are cleared

When a player presses `Space`, `fire_sudden_death_shot` tries to create a bullet.

The shot only fires if:

- ammo is greater than `0`
- there is an unused bullet slot

Each player has a fixed bullet pool of `5`. This keeps the code simple and avoids dynamic memory.

Each active bullet stores:

- row
- column
- active flag
- movement timer

Bullets move over time in `game_update_bullets`. If a bullet reaches the enemy row and the column matches, it deals `10` damage.

### Why sudden death bullets use arrays

Originally, normal resolve only needed one bullet per player because each player can shoot once per round.

Sudden death allows repeated shooting, so one bullet slot is not enough.

The sudden death arrays solve that:

- `p1_sd_bullet_row`
- `p1_sd_bullet_col`
- `p1_sd_bullet_active`
- `p1_sd_bullet_step_ms`

Player 2 has the same set.

The arrays are fixed size because ammo is fixed size. This is simple, predictable, and fits the game.

### How damage feedback works

Damage feedback uses four fields:

- `p1_damage_feedback`
- `p2_damage_feedback`
- `p1_damage_feedback_ms`
- `p2_damage_feedback_ms`

When damage is applied, the game stores the amount and starts a short timer.

During rendering, the player marker stays fixed as `1` or `2`. The `-10HP` text is drawn in a neighboring arena cell. It is not drawn inside the player cell because that would change the cell width and break the arena border.

In sudden death, feedback appears only after the bullet actually reaches the target and hits. It does not appear when the bullet is fired.

### How the arena stays aligned

Every arena lane prints exactly seven characters.

Examples:

- empty cell: seven spaces
- player: three spaces, player number, three spaces
- bullet: three spaces, bullet symbol, three spaces
- damage text: padded to exactly seven characters

The arena border depends on this. If one cell prints six or eight characters, the right border shifts. That is why every cell string must stay the same width.

### How the header stays aligned

The header uses a fixed-width box.

Each side of the header has the same visible width. The code prints:

- left border
- one space
- exactly fixed-width content
- one space
- divider or right border

Color escape codes do not count as visible characters, so the code manually tracks visible width for HP bars, potion bars, and ammo bars.

If a row prints one extra visible character, the divider moves. If it prints one too few, the divider appears too early. That is why the HP row has its own careful width tracking.

### How the state packet works

The server serializes the full game state as one text line starting with `STATE`.

The packet is not binary. It is a series of integers separated by spaces.

The client parses those integers in the same order.

This is simple and easy to debug, but it has one rule:

If the server adds, removes, or reorders fields, `parse_state_line` in `client.c` must be updated the same way.

### Why line buffers matter

The state packet became longer after sudden death bullet arrays were added.

The client line buffer must be large enough to hold the whole packet. If it is too small, the client reads only part of the `STATE` line and parsing fails. When parsing fails, the client does not render the game.

That is why the client uses `NET_BUFFER_SIZE` for incoming state lines.

### How quitting works

Quit uses a two-press confirmation.

First `q`:

- arms quit
- renders `Press q again to quit`

Second `q`:

- exits directly if on the start screen
- exits directly if on game over
- forfeits if inside an active match

This prevents accidental quits during play while still making the start screen exit cleanly.

## Network Protocol

The protocol is line-based text.

### Client to server

`LEFT`

- Move left in `MOVE`.
- Select `SHOOT` in `ACTION`.
- Vote `YES` in sudden death offer.
- Move left in sudden death battle.

`RIGHT`

- Move right in `MOVE`.
- Select `HEAL` in `ACTION`.
- Vote `NO` in sudden death offer.
- Move right in sudden death battle.

`LOCK`

- Ready on start screen.
- Lock position.
- Lock action.
- Lock sudden death vote.
- Ready for sudden death.
- Shoot during sudden death battle.

`QUIT`

- Quit or forfeit depending on phase.

### Server to client

The server sends one `STATE` line containing the replicated `GameState`.

The packet includes:

- phase and phase timer
- round and winner
- both players' HP, positions, potions, actions, locks, and ready flags
- normal bullet animation state
- sudden death vote state
- sudden death ammo and reload state
- damage feedback
- sudden death bullet pools

If the server packet changes, the client parser must be updated to match.

## Rendering Notes

The game uses terminal escape sequences:

- `\033[?1049h`: enter alternate screen
- `\033[?1049l`: leave alternate screen
- `\033[?25l`: hide cursor
- `\033[?25h`: show cursor
- `\033[2J\033[H`: clear screen and move cursor home

The renderer also uses a display key so it does not redraw identical frames.

## Current Behavior Summary

- Start screen shows only the `WILD WEST` banner.
- Normal and sudden death arenas use the same height.
- Enemy position is hidden during `MOVE` and `ACTION`.
- Enemy action is hidden during `ACTION`.
- Enemy position is visible during `RESOLVE`, `SUDDEN_DEATH_BATTLE`, and `GAME_OVER`.
- Damage feedback appears only after damage is actually applied.
- Quitting from the start screen exits directly.
- Quitting during an active match forfeits.
- Quitting from game over exits.
