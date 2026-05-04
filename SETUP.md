# Wild West Quick Draw Setup

## Requirements

- Ubuntu/Linux or macOS
- `gcc`
- For network play: both PCs on the same network
- The server machine must allow inbound TCP traffic on the chosen port

## Build

```bash
cd wildwest
make
```

This builds:

- `./server`
- `./client`

## Local Run On One Mac

Terminal 1:

```bash
cd "/Users/austin/Documents/New project/wildwest"
./server
```

Terminal 2:

```bash
cd "/Users/austin/Documents/New project/wildwest"
./client 127.0.0.1
```

## Network Run On Two PCs

### PC 1: Server

Find the server IP:

```bash
ip addr
```

On macOS:

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

Connect using the server IP:

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

## Controls

- `Left Arrow` = move left in move phase
- `Right Arrow` = move right in move phase
- `[` = select `SHOOT` in action phase
- `]` = select `HEAL` in action phase
- `Space` = lock / commit
- `q` = quit

## Match Flow

0. Ready phase
- Press `Space` to mark yourself ready
- The match starts only when both players are ready

1. Move phase
- 15 seconds to choose and lock a position
- Enemy position is hidden
- Press `Space` to lock your position early

2. Action phase
- 10 seconds to choose an action
- Enemy position is still hidden
- `[` selects `SHOOT`
- `]` selects `HEAL`
- Press `Space` to lock your action early

3. Resolve phase
- Enemy position is revealed
- Both locked actions resolve
- Shot hit deals `20` damage
- Critical hit deals `30` damage
- Miss deals `10` self-damage
- Heal restores `30` HP and uses `1` potion
- Each player starts with `3` potions

## Notes

- Same port number alone does not connect two machines.
- The client must use the server machine IP address and the same port.
- If the client cannot connect, check firewall rules and confirm both machines are on the same network.
