# Wild West Quick Draw

Wild West Quick Draw is a two-player terminal duel written in C. One terminal runs the server for Player 1. Another terminal runs the client for Player 2. The server owns the real match state and the client renders whatever state the server sends.

The game is built for local terminal play or two machines on the same network.

## Current Gameplay

Each round has three normal phases:

- `MOVE`: both players choose a lane while the enemy position is hidden.
- `ACTION`: both players choose `SHOOT` or `HEAL` while the enemy action is hidden.
- `RESOLVE`: positions are revealed and the locked actions resolve.

If both players drop to `20` HP or below and no winner exists yet, the game asks both players if they want sudden death. Sudden death only starts if both vote `YES`.

## Controls

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

- `Left Arrow` = `YES`
- `Right Arrow` = `NO`
- `Space` = lock vote

Sudden death battle:

- `Left Arrow` = move left
- `Right Arrow` = move right
- `Space` = shoot

During the match, `q` twice quits. In game-over, `q` twice exits the result screen.

## Combat Rules

- Normal shot hit: `20` damage
- Critical hit: `30` damage
- Miss backfire: `10` self-damage
- Heal: restores `30` HP
- Potions: each player starts with `3`
- Sudden death bullet damage: `10`
- Sudden death ammo: `5`, reloads after `2` seconds when empty

Damage feedback appears beside a player after damage is actually applied. In sudden death, that means the `-10HP` text appears only when the bullet reaches and hits the player.

## Build

```bash
make
```

This builds:

- `./server`
- `./client`

## Run On One Machine

Terminal 1:

```bash
./server
```

Terminal 2:

```bash
./client 127.0.0.1
```

## Run On Two Machines

On the server machine:

```bash
./server
```

On the client machine:

```bash
./client <server-ip>
```

Both machines must be on the same network, and the client must connect to the server machine IP address.

## Project Structure

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

## Notes

- The game uses TCP sockets.
- The server is authoritative.
- The client does not simulate gameplay decisions.
- No external runtime dependency is needed for the title or result banners. `figlet` was used during development, then the generated banner text was adapted into C strings.

## Developers

- Jorge Creiann Jarme
- Karl Austin Pavia
- Jose Miguel Villareal
