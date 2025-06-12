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


---
