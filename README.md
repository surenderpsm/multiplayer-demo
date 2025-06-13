# Multiplayer State Sync Demo (C++ / UDP)

This is a server-authoritative multiplayer simulation demo built in C++ using raw UDP sockets. The goal is to explore real-time state synchronization between multiple clients and a centralized server, handling latency, packet loss, and stress conditions.

## 🚩 Key Components

### 🔹 **Server** (`server/server.cpp`)
- Opens a UDP socket on port `9000`.
- Uses `GameManager` to:
  - Process incoming messages
  - Broadcast game state every 100 ms
- Main loop:
  - Receives packets with `recvfrom()`
  - Passes messages to `GameManager::handleMessage()`
  - Sends responses if needed

---

### 🔸 **GameManager** (`server/game_manager.{h,cpp}`)
- Manages game lifecycle:  
  `UNKNOWN → WAITING → STARTED → ENDED`
- Handles:
  - `HELLO` handshake  
  - `PING` keep-alive  
  - `UPDATE` position messages
- Delegates to `ClientManager` for:
  - Client tracking
  - ID validation
  - Position updates
  - Inactivity pruning
- Periodically checks to:
  - Start game (when player count or wait time condition is met)
  - End game (if player count drops below minimum)

---

### 🧱 **ClientManager** (`server/client_manager.{h,cpp}`)
- Maintains a map of connected clients (keyed by IP:port)
- Handles:
  - `UPDATE:<id>:<x>:<y>` — validates and updates position
  - `PING:<id>` — updates last seen timestamp
- Broadcasts combined game state:  
  ```
  STATE:<id1>:<x1>:<y1>|<id2>:<x2>:<y2>|...
  ```
- Removes clients inactive for more than `CLIENT_TIMEOUT_MS` (default: 5000 ms)

---

### 📽️ **Client** (`client/client.cpp`)
- Sends `HELLO` to register with server
- Receives `WELCOME:<id>` and stores assigned ID
- Listens for game state via `GAME:<state>|STATE:...`
- Behaviour:
  - During `STARTED`: sends `UPDATE` messages every 100 ms
  - During `WAITING`: sends `PING` keep-alives every 100 ms

---

### ⚙️ **Configuration** (`common/config.h`)
- Central config file for tuning game constants:
  - `MIN_PLAYERS` — minimum to start game
  - `MAX_PLAYERS` — maximum allowed
  - `WAIT_TIME_SEC` — countdown before starting
  - `CLIENT_TIMEOUT_MS` — time to drop inactive clients

---

### 🧪 **Testing Scripts** (`test/`)
- Python scripts for:
  - Handshake-only client
  - Spoofing invalid `UPDATE` packets
  - Stress-testing with multiple fake clients


## Repository Overview

### Phase 1: Core network scaffold
- Basic UDP server that listens on port `9000`
- UDP client that sends random `(x, y)` position every 100ms
- Echo loop: server receives and sends back message
- Client logs sent and received positions
```
                  +-------------------+
                  |     Server        |
                  |   (server.cpp)    |
                  +---------+---------+
                            ^
                            |
      +---------------------+---------------------+
      |                     |                     |
+-------------+     +-------------+       +-------------+
|  Client A   |     |  Client B   |  ...  |  Client N   |
| (client.cpp)|     | (client.cpp)|       | (client.cpp)|
+-------------+     +-------------+       +-------------+

Each client sends position updates to the server every 100ms.
The server echoes received positions back to each client.
Planned expansion: rebroadcast all player states to all clients.
```

### Phase 2: Client Management and State Broadcast

#### 2.1 Client Registration (Handshake)
- Clients initiate connection with a "HELLO" message.
- Server responds with a "WELCOME:<id>" message assigning a unique client ID.
- Clients are tracked internally by their IP:Port and assigned ID.

#### 2.2 Position Updates and Broadcast
- Registered clients periodically send position updates in the format:  
  `"UPDATE:<id>:<x>:<y>"`
- The server validates the sender using their IP:Port and assigned ID.
- Every 100ms, the server broadcasts the global state of all active clients in the format:  
  `"STATE:<id1>:<x1>:<y1>|<id2>:<x2>:<y2>|..."`

#### 🔐 Packet Validation
- The server discards updates from unregistered clients or mismatched IDs to prevent spoofing.
- All clients are maintained in an internal map with their last known position and activity timestamp.

#### ✅ Tested functionality
- Valid client handshake and subsequent updates
- Invalid client ID with no handshake
- Valid client ID trying to spoof from another IP is dropped by the server

---

### Phase 3: Game Lifecycle Management

#### 3.1 GameManager Integration
- Introduced `GameManager` to manage the game state lifecycle.
- States: `WAITING`, `STARTED`, `ENDED`
- Clients can only join in the `WAITING` state.
- Game starts when max players are reached or wait time expires.

#### 3.2 Server Refactoring
- All client handling logic moved into `GameManager::handleMessage()`
- Server delegates state broadcasting to `GameManager::broadcastToAll()`
- Game state updates tracked via `GameManager::update()` every tick

#### 📦 Broadcast Format
```
GAME:<state>|ID1:X1:Y1;ID2:X2:Y2;...
```

#### 🧱 Server Architecture Diagram
```
+------------------------+
|      Server Main       |
|    (server.cpp)        |
+-----------+------------+
            |
            v
+------------------------+
|     GameManager        |
+-----------+------------+
            |
            v
+------------------------+
|    ClientManager       |
+------------------------+
```

- `Server` runs the loop, delegates message handling to `GameManager`
- `GameManager` manages state transitions and delegates to `ClientManager`
- `ClientManager` handles client registry and state

---

### Phase 3.5: Game Lifecycle Improvements: keep-alive and refactoring

#### 3.1 GameManager Updates
- States: `UNKNOWN`, `WAITING`, `STARTED`, `ENDED`
- Game starts when max players joined or wait timeout hits.
- Game ends if players drop below `MIN_PLAYERS` during play.

#### 3.2 Client Keep-Alive
- During `WAITING`, clients send `PING:<id>` to remain active.
- Server resets inactivity timer based on `PING` or `UPDATE`.

#### 3.3 Server Refactoring
- `server.cpp` delegates everything to `GameManager::handleMessage()`.
- Broadcasts done via `GameManager::broadcastToAll()`.
- Prunes inactive clients every update tick.

#### 3.4 General Refactoring
- added `common/config.h` to tweak game parameters.

---

## 🛣️ Next Steps
- Implement player actions like shooting
- Handle disconnections / timeouts
- Support game over and reset states

---
