# Wild West Quick Draw 🎯

A turn-based multiplayer duel game set on a vertical battlefield where players predict movement, manage resources, and outplay opponents using strategy, timing, and deception.

---

## 🕹️ Game Overview

Two players start on opposite ends of a vertical grid. Each turn, players choose to:

- Move (up or down)
- Shoot (deal damage if aligned on same row)
- Heal (use limited potions)

The first player to reduce the opponent’s HP to 0 wins.

---

## ⚙️ Features

- Multiplayer real-time server using C (sockets + threads)
- Turn-based combat system
- OOP-style architecture in C (structs + function pointers)
- Modular codebase (src/include separation)
- Hybrid mode with AI fallback
- Python AI using Markov Chain prediction model
- Smart opponent behavior based on movement patterns

---

## 🧠 AI System (Python)

The AI uses a **Markov Chain model** to predict enemy movement:

- Learns transition probabilities between rows
- Predicts next opponent position
- Adapts over time during gameplay
- Uses stochastic decision-making for unpredictability

---
## 🏗️ Project Structure (Not Final)
```bash
wildwest
├── LICENSE
├── ai
│   ├── ai_player.py
│   ├── base_player.py
│   └── strategy.py
├── include
│   ├── game.h
│   ├── network.h
│   ├── player.h
│   └── server.h
├── makefile
├── net
│   └── client.py
└── src
    ├── game.c
    ├── main.c
    ├── network.c
    ├── player.c
    └── server.c
```


---


## 🔌 Architecture


Browser (JS)
↓
WebSocket / Node bridge (optional)
↓
C Server (core multiplayer engine)
↓
Python AI (Markov-based opponent, optional fallback)


---

## 🚀 Build & Run

### Compile C server
```bash
make
./server
```

Run Python AI:
```bash
cd python
python main.py
```

---



🎓 Concepts Demonstrated:
- Socket programming (C)
- Multithreading (pthread)
- Modular software design
- OOP simulation in C
- True OOP in Python
- Markov Chain AI
- Hybrid PvP / PvE system design


---


📌 Notes:
- Designed for Operating Systems coursework
- Focus on concurrency, modularity, and system-level design
- AI is optional and runs independently of core server
