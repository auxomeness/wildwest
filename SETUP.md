# Wild West Quick Draw Setup

## Requirements

- Ubuntu/Linux or macOS
- `gcc`
- `make`
- For two-machine play: both machines must be on the same network
- The server machine must allow inbound TCP traffic on the chosen port

No `figlet` install is required. `figlet` was only used during development to generate the banner text. The generated banners were copied and adapted into C string arrays, so the game can build and run without installing `figlet`.

## Build

From the project folder:

```bash
make
```

This creates:

- `./server`
- `./client`

If you want to rebuild everything from scratch:

```bash
make -B
```

## Run On One Mac Or One Linux Machine

Open two terminal windows.

Terminal 1:

```bash
cd path/to/wildwest
./server
```

Terminal 2:

```bash
cd path/to/wildwest
./client 127.0.0.1
```

Replace `path/to/wildwest` with the actual folder path where you downloaded or cloned the project.

## Run On Two Networked Machines

### PC 1: Server

Find the server IP address.

Ubuntu/Linux:

```bash
ip addr
```

macOS:

```bash
ifconfig
```

Start the server:

```bash
./server
```

Optional custom port:

```bash
./server 51717
```

### PC 2: Client

Connect using the server machine IP address:

```bash
./client <server-ip>
```

Example:

```bash
./client 192.168.1.25
```

Optional custom port:

```bash
./client 192.168.1.25 51717
```

The client must use the same port as the server.

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

- `Left Arrow` = vote `YES`
- `Right Arrow` = vote `NO`
- `Space` = lock vote

Sudden death battle:

- `Left Arrow` = move left
- `Right Arrow` = move right
- `Space` = shoot

General:

- `q` twice = quit

## Match Flow

1. Start screen

- Both players press `Space`.
- The game starts only when both players are ready.
- If you quit here, the program exits directly without showing the arena.

2. Move phase

- 15 seconds to choose and lock a lane.
- Enemy position is hidden.
- Press `Space` to lock early.

3. Action phase

- 10 seconds to choose `SHOOT` or `HEAL`.
- Enemy action is hidden.
- Press `Space` to lock early.

4. Resolve phase

- Enemy position is revealed.
- Locked actions resolve.
- Shot hit deals `20` damage.
- Critical hit deals `30` damage.
- Miss deals `10` self-damage.
- Heal restores `30` HP and uses `1` potion.

5. Sudden death offer

- If both players are at `20` HP or below and no winner exists, both players vote.
- Both must vote `YES` to enter sudden death.
- If either player votes `NO`, the normal game continues.

6. Sudden death ready

- HP resets to `100`.
- Ammo resets to `5`.
- Both players press `Space` when ready.

7. Sudden death battle

- Both players are visible.
- Players move left or right and shoot with `Space`.
- Bullets deal `10` damage.
- Ammo reloads to `5` after `2` seconds when empty.
- First player to kill the opponent wins.

## Troubleshooting

- If the client cannot connect, confirm both machines are on the same network.
- Confirm the client is using the server machine IP address.
- Confirm both sides use the same port.
- Check firewall settings on the server machine.
- If terminal rendering looks broken, increase the terminal window size.
