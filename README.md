# Multiplayer State Sync Demo (C++ / UDP)

This is a server-authoritative multiplayer simulation demo built in C++ using raw UDP sockets. The goal is to explore real-time state synchronization between multiple clients and a centralized server, handling latency, packet loss, and stress conditions.

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

## 🛣️ Next Steps
- Implement player actions like shooting
- Handle disconnections / timeouts
- Support game over and reset states

---
