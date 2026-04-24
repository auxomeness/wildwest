# Wild West Quick Draw Documentation

## Overview

This project is a two-player terminal duel written in C. One process runs as the server, one process runs as the client. The server owns the real game state, timers, phase changes, damage, healing, crit checks, and winner detection. The client does not simulate the match by itself. It sends input commands to the server and renders the latest state snapshot it receives back.

The match is phase-based:

1. `WAITING`
- Both players press `Space` to mark themselves ready.
- The game does not start until both sides are ready.

2. `MOVE`
- Players have 15 seconds to choose a lane.
- Enemy position is hidden.
- `Left Arrow` and `Right Arrow` move the player.
- `Space` locks the position early.

3. `ACTION`
- Players have 10 seconds to choose an action.
- Enemy position is still hidden.
- `[` selects `SHOOT`.
- `]` selects `HEAL`.
- `Space` locks the action early.

4. `RESOLVE`
- Enemy position is revealed.
- Locked actions are applied.
- Shots can hit, crit, or miss.
- Misses backfire and can kill the shooter.

5. `GAME OVER`
- The winner or draw is shown.

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
├── SETUP.md
└── DOCUMENTATION.md
```

### What each area is for

`include/`
- Header files.
- These define shared constants, structs, enums, and function declarations.
- The `.c` files include these headers so the compiler knows what each module exposes.

`src/`
- Source files.
- This is where the implementation lives.

`makefile`
- Build rules for `server` and `client`.

`SETUP.md`
- Run instructions.

`DOCUMENTATION.md`
- This document.

## Build Layout

The project builds two binaries:

`server`
- Compiled from `src/main.c`, `src/server.c`, and the shared source files.
- Runs the authoritative match loop.

`client`
- Compiled from `src/client.c` and the shared source files.
- Connects to the server and renders the remote match.

The `makefile` uses `-Iinclude`, which tells the compiler to search the `include/` folder for headers. That is why the source code uses:

```c
#include "game.h"
```

and not:

```c
#include "./include/game.h"
```

## Runtime Architecture

The match is server-authoritative.

### Server responsibilities

- Accept one client connection.
- Read local keyboard input for Player 1.
- Read remote commands from Player 2.
- Advance timers.
- Switch phases.
- Resolve shots, heals, crits, and misses.
- Detect winners.
- Send the full game state to the client.

### Client responsibilities

- Read local keyboard input for Player 2.
- Send text commands to the server.
- Receive the latest game state from the server.
- Render the state exactly as received.

### Why this split matters

Only one process decides what is true. That avoids desync between players. The client is a terminal frontend with input handling and rendering, not a second game simulation.

## Header Files

### `include/player.h`

This file defines player-specific state and the result labels used during resolve.

#### Enums

`Action`
- `ACTION_NONE`
- `ACTION_SHOOT`
- `ACTION_HEAL`

Used during the action phase to describe what a player locked in.

`ResolveResult`
- `RESULT_NONE`
- `RESULT_SHOT_HIT`
- `RESULT_SHOT_CRIT`
- `RESULT_SHOT_MISS`
- `RESULT_HEAL`
- `RESULT_HEAL_FAIL`

Used to describe what happened after actions were resolved.

#### Struct

`Player`
- `col`
  Current horizontal position inside the arena.
- `hp`
  Current health.
- `potions`
  Remaining heals.
- `crit_chance`
  Percent chance for a critical hit.
- `action`
  Current selected action for the action phase.
- `locked`
  Whether the player already locked input for the current phase.

#### Functions

`player_init`
- Sets the starting stats for a player.

`player_move`
- Moves left or right but clamps the result inside the arena width.

`player_lock` / `player_unlock`
- Marks the player as done or not done for the current phase.

`player_clear_action`
- Resets action to `ACTION_NONE`.

`player_set_action`
- Sets the selected action to `SHOOT` or `HEAL`.

`player_apply_shot`
- Resolves a shot against a target.
- Hit deals 20 damage.
- Crit deals 30 damage.
- Miss deals 10 self-damage.
- A miss can kill the shooter.

`player_apply_heal`
- Resolves healing.
- Only works if the player has potions left and is not already at full HP.

### `include/game.h`

This file defines all shared game constants and the full replicated match state.

#### Important constants

`DEFAULT_PORT`
- Default TCP port.

`GRID_HEIGHT`
- Current visible arena height.

`GRID_WIDTH`
- Number of lanes.

`PLAYER1_ROW`
- Row used by Player 1.

`PLAYER2_ROW`
- Row used by Player 2.

`MAX_HP`
- Maximum health cap.

`MAX_POTIONS`
- Starting and maximum number of potions.

`SHOT_DAMAGE`
- Normal hit damage.

`CRIT_DAMAGE`
- Critical hit damage.

`MISS_DAMAGE`
- Self-damage on a missed shot.

`HEAL_AMOUNT`
- Health restored by a successful heal.

`DEFAULT_CRIT_CHANCE`
- Starting critical-hit chance.

`MOVE_PHASE_MS`
- Move timer in milliseconds.

`ACTION_PHASE_MS`
- Action timer in milliseconds.

`RESOLVE_PHASE_MS`
- Resolve animation time in milliseconds.

#### Enum

`Phase`
- `PHASE_WAITING`
- `PHASE_MOVE`
- `PHASE_ACTION`
- `PHASE_RESOLVE`
- `PHASE_GAME_OVER`

This tells both terminals which part of the match is active.

#### Struct

`GameState`
- `p1`, `p2`
  Full state for both players.
- `p1_ready`, `p2_ready`
  Ready-up flags before the duel starts.
- `p1_result`, `p2_result`
  Result labels for the last resolve.
- `bullet1_row`, `bullet1_col`, `bullet1_active`
  Temporary animation state for Player 1's shot.
- `bullet2_row`, `bullet2_col`, `bullet2_active`
  Temporary animation state for Player 2's shot.
- `phase`
  Current game phase.
- `phase_time_ms`
  Time elapsed in the current phase.
- `round_number`
  Current round number.
- `winner`
  `0` means no winner yet, `1` means Player 1, `2` means Player 2, `3` means draw.
- `running`
  Whether the match is still active.

#### Functions

`game_init`
- Creates the initial waiting state.

`game_start_move_phase`
- Resets round state and starts a new move phase.

`game_start_action_phase`
- Opens the action phase and defaults both actions to `SHOOT`.

`game_start_resolve_phase`
- Applies actions and begins the resolve animation.

`game_update_bullets`
- Advances bullet sprites while in resolve.

`game_phase_time_limit`
- Returns the duration for the given phase.

`game_countdown_seconds`
- Converts remaining milliseconds into a user-facing countdown number.

`game_phase_label`
- Returns the text label for a phase.

`game_action_label`
- Returns the text label for an action.

`game_result_label`
- Returns the text label for a resolve result.

`game_build_display_key`
- Produces a compact summary of the current visible frame.
- Used to avoid re-rendering identical screens.

`game_render`
- Draws one full terminal frame from the perspective of one player.

### `include/network.h`

This file contains the socket helper interface.

#### Struct

`NetBuffer`
- `data`
  Raw byte buffer for incoming text.
- `used`
  Number of bytes currently stored.

#### Functions

`create_server`
- Opens, binds, and listens on a TCP socket.

`accept_client`
- Waits for one client connection.

`create_client`
- Connects to the server by IP address and port.

`set_nonblocking`
- Turns a socket into nonblocking mode.

`send_all`
- Keeps sending until the whole buffer has been written.

`send_text`
- Convenience wrapper for sending null-terminated text.

`net_buffer_init`
- Clears the input buffer.

`net_read_into_buffer`
- Reads raw bytes from a socket into the buffer.

`net_next_line`
- Extracts one newline-terminated message from the buffer.

### `include/server.h`

This file only exposes one function:

`start_server`
- Starts the server process and enters the full authoritative game loop.

## Source Files

### `src/main.c`

This is the entry point for the server binary.

#### How it runs

1. Start with `DEFAULT_PORT`.
2. If the user passed a numeric argument, use that as the port.
3. Call `start_server(port)`.

This file stays small on purpose. All real match logic is kept in `server.c`.

### `src/player.c`

This file handles player-specific rules.

#### `player_init`

Sets:
- starting column
- 100 HP
- 3 potions
- default crit chance
- no action selected
- unlocked state

#### `player_move`

Applies horizontal movement and clamps it between the minimum and maximum lane.

#### `player_apply_shot`

Rules:
- If the player did not choose `SHOOT`, nothing happens.
- If both players are in the same column:
  - roll crit chance
  - crit deals 30
  - normal hit deals 20
- If the shot misses:
  - shooter takes 10 backfire damage
  - this can reduce HP to 0

#### `player_apply_heal`

Rules:
- Only works if action is `HEAL`
- Fails if:
  - player is already dead
  - no potions remain
  - HP is already full
- Success:
  - heal 30
  - clamp to 100
  - spend one potion

### `src/game.c`

This file owns the gameplay state transitions and the renderer.

#### Internal helpers

`clear_bullets`
- Clears bullet animation state.

`clear_results`
- Clears last resolve messages.

`update_winner`
- Evaluates the winner after actions are applied.

`local_player`, `enemy_player`
- Return which `Player` struct should be treated as self or enemy for the current terminal.

`local_result`, `enemy_result`
- Same idea, but for resolve results.

#### Match lifecycle

`game_init`
- Builds the initial waiting screen.
- No round starts yet.

`game_start_move_phase`
- Clears old actions, results, and bullet trails.
- Unlocks both players.
- Resets ready flags.
- Starts the next round and increments `round_number`.

`game_start_action_phase`
- Defaults both actions to `SHOOT`.
- Unlocks both players.
- Starts the action timer.

`game_start_resolve_phase`
- Applies the locked actions using the locked positions.
- Spawns bullet animations if a player chose `SHOOT`.
- Applies shot damage and heal logic.
- Updates the winner flag.

`game_update_bullets`
- Moves bullet sprites one row per update during resolve.

#### Rendering

`game_build_display_key`
- Builds a visibility key based on the current player perspective.
- The server and client use this to skip redundant redraws.

`game_render`
- Clears the terminal frame.
- Prints the header.
- Prints the boxed arena.
- Hides enemy position outside of resolve and game over.
- Prints phase-specific instructions below the arena.
- Prints the action selector below the frame during action phase.

### `src/network.c`

This file handles socket setup and line-based transport.

#### `create_server`

Creates a TCP socket and prepares it to accept one client.

Important system calls:
- `socket`
- `setsockopt`
- `bind`
- `listen`

#### `create_client`

Creates a TCP socket and connects to the server IP and port.

Important system calls:
- `socket`
- `inet_pton`
- `connect`

#### `set_nonblocking`

Uses `fcntl` so reads and writes do not stall the game loop.

#### `send_all`

Handles partial writes. This matters because socket sends are not guaranteed to write the whole buffer in one call.

#### `net_read_into_buffer`

Reads raw socket bytes into `NetBuffer` and keeps enough state to handle partial lines.

#### `net_next_line`

Extracts one line ending in `\n`, copies it out, and compacts the buffer.

This is why the protocol is line-based instead of binary.

### `src/server.c`

This is the main runtime for the server binary.

#### Terminal handling

`enable_raw_mode`
- Disables canonical input and echo.
- Enters the alternate screen buffer.
- Hides the cursor.

`restore_terminal`
- Restores cooked mode.
- Returns to the normal terminal screen.
- Shows the cursor again.

This is what prevents frame-by-frame rendering from polluting scrollback while the game is running.

#### Input handling

`key_available`
- Checks whether a key is waiting without blocking.

`read_key`
- Converts raw input bytes into input tokens.

Accepted keys:
- `Left Arrow`
- `Right Arrow`
- `Space`
- `q`
- `[`
- `]`

#### State serialization

`send_state`
- Packs the current `GameState` into one text line.
- Sends it to the client every server tick.

#### Input application

`handle_local_input`
- Applies Player 1 input directly to the authoritative state.

`handle_remote_command`
- Applies Player 2 commands that arrived from the client.

#### Match control

`update_match`
- Controls phase transitions:
  - waiting to move when both are ready
  - move to action on both locks or timer expiry
  - action to resolve on both locks or timer expiry
  - resolve to next round or game over after the resolve timer

#### Main loop

`start_server`

Flow:

1. Ignore `SIGPIPE`
2. Seed the random number generator for crits
3. Create the listening socket
4. Accept one client
5. Set the client socket to nonblocking
6. Enable raw terminal mode
7. Initialize game state
8. Loop:
   - compute frame delta
   - read local keyboard input
   - read remote commands
   - update the match
   - send state to the client
   - render only when the visible frame changed

### `src/client.c`

This is the runtime for the remote player.

It mirrors the terminal behavior of the server:
- raw mode
- alternate screen
- hidden cursor
- nonblocking keyboard polling

#### `render_connecting_screen`

Shows a temporary screen before the client receives the first full state.

#### `parse_state_line`

Parses the server's text packet back into a local `GameState`.

The client does not decide what the game state should be. It only displays what the server sent.

#### `main`

Flow:

1. Read server IP and optional port from the command line
2. Connect to the server
3. Set the socket to nonblocking
4. Enable raw mode and alternate screen
5. Loop:
   - read keyboard input
   - convert it into text commands
   - send commands to the server
   - receive state packets
   - parse the latest packet
   - render only when the visible frame changed

## Network Protocol

The protocol is text-based.

### Client to server commands

`LEFT`
- Move left during move phase.

`RIGHT`
- Move right during move phase.

`SHOOT`
- Select shoot during action phase.

`HEAL`
- Select heal during action phase.

`LOCK`
- Ready up in waiting phase.
- Lock position in move phase.
- Lock action in action phase.

`QUIT`
- Leave the match.

### Server to client state packet

The server sends one line beginning with `STATE` followed by the fields needed to rebuild `GameState`.

This packet includes:
- phase
- phase timer
- round
- winner
- running flag
- both players' HP
- both players' columns
- both players' potion counts
- both players' actions
- both lock flags
- both ready flags
- both resolve results
- bullet animation state

## Variables That Matter Most

### Player fields

`col`
- Current lane.

`hp`
- Health.

`potions`
- Remaining heals.

`crit_chance`
- Used during shooting to decide whether a hit becomes a critical hit.

`action`
- Selected action for the current action phase.

`locked`
- Whether the player already committed the current phase.

### GameState fields

`phase`
- Current game mode.

`phase_time_ms`
- Elapsed time inside the current phase.

`round_number`
- Current round count.

`winner`
- Winning side or draw code.

`running`
- Whether the match is still active.

`p1_ready`, `p2_ready`
- Ready flags on the waiting screen.

`p1_result`, `p2_result`
- Labels shown during resolve.

`bullet*_row`, `bullet*_col`, `bullet*_active`
- Only used for resolve animation.

## Why The Screen Now Behaves Properly

Two separate changes matter here.

First, both binaries use the terminal's alternate screen buffer:
- enter with `\033[?1049h`
- leave with `\033[?1049l`

That keeps the game UI out of the normal terminal scrollback while it is running.

Second, both binaries build a display key and only redraw when the visible frame changes.

That reduces useless redraw spam and makes the terminal output much easier to follow while still keeping the countdown live.

## Current Controls

### Waiting phase

- `Space` = ready
- `q` = quit

### Move phase

- `Left Arrow` = move left
- `Right Arrow` = move right
- `Space` = lock
- `q` = quit

### Action phase

- `[` = select shoot
- `]` = select heal
- `Space` = lock
- `q` = quit

## Current Rules

- Enemy position is hidden during `WAITING`, `MOVE`, and `ACTION`
- Enemy position becomes visible during `RESOLVE`
- Move phase lasts 15 seconds
- Action phase lasts 10 seconds
- Resolve phase lasts 2 seconds
- Hit deals 20 damage
- Crit deals 30 damage
- Miss deals 10 self-damage
- Miss backfire can kill the shooter
- Heal restores 30 HP
- Heal uses one potion
- Each player starts with 3 potions

If one side changes the packet but the other side does not, the client and server will stop understanding each other.
