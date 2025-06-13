# Multiplayer UDP Game Simulation and Stress Testing

## Overview

This project simulates a real-time multiplayer game server using UDP sockets, focusing on network state synchronization, packet handling, and performance under high concurrency. A custom protocol enables clients to join, exchange position updates, and receive periodic world state broadcasts tagged with a server-side tick. The project includes a stress test framework that simulates up to 1000 clients to analyze packet loss and message throughput.

The implementation is optimized for clarity and performance, using multithreading to handle concurrent client updates and regular broadcasts.

## Architecture

* **Language**: C++ for the server, Python for the stress test clients.
* **Transport Protocol**: UDP (no built-in reliability, packet ordering, or retransmission).
* **Server**:

  * Listens for client connections on port 9000.
  * Handles initial handshakes via `HELLO` / `WELCOME:<id>`.
  * Maintains a tick counter, incremented every 100ms.
  * Broadcasts current game state to all clients on every tick (via `GAME:STATE:TICK=<n>`).
* **Client (Stress Test)**:

  * Spawns multiple threads simulating different clients.
  * Each client sends position updates every 100ms and listens for `GAME:STATE` messages.
  * Tracks tick coverage to compute packet loss.

## Features Implemented

* Stateless UDP handshake and ID assignment.
* Tick-based simulation loop (100ms interval).
* Server broadcasts include a tick tag to measure delivery accuracy.
* Multithreaded stress test clients with loss detection metrics.
* Command-line configuration for test duration and client count.

## Problem Statement

The goal is to benchmark a basic real-time UDP multiplayer server's ability to handle high player loads (100+ concurrent clients) and measure packet reliability in absence of TCP-style guarantees. The stress test evaluates how many state update packets are lost under various loads and MTU constraints.


## 📊 Network Performance Graphs

### 1. Packet Loss vs Client Count

![Packet Loss Graph](graphs/candlestick_packet_loss.png)

This chart visualizes the relationship between client concurrency and packet loss during server broadcasts. Each red bar represents the *mean packet loss* across all clients for a given client count, and the black whiskers indicate the *minimum and maximum loss observed*.

- *Below 800 clients*: Packet loss is negligible (≈ 0%).
- *Between 900–1000 clients*: Loss increases modestly, indicating UDP limits under stress.
- *1050+ clients*: Some clients experience >30% loss, suggesting socket buffer saturation or exceeding the network interface's capacity to handle large concurrent sends.

---

### 2. Broadcast Packet Size vs Client Count

![Packet Size Graph](graphs/candlestick_packet_size_naive.png)

This plot shows how the *average packet size (in bytes)* increases as the number of connected clients grows, due to naive string-based serialization of player state.

- Packets approach and exceed *4000 bytes* around 800–900 clients.
- Beyond 1000 clients, most packets likely exceed the typical UDP MTU (~1472 bytes for IPv4 over Ethernet), causing fragmentation and potential loss.

> 🧠 *Insight: The server does not experience logic bottlenecks, but suffers from **UDP's lack of flow control and the OS's limited buffer size*. The tests show the tipping point for UDP saturation under naive payload growth.

## Future Work

* Implement reliable delivery via selective retransmission.
* Protobuf serialization for compact, consistent packet structures.
* Add basic gameplay: health, shooting, and kill logic.
* Implement congestion control or adaptive tick intervals.
* Optional: Replace Python clients with C++ ones for tighter integration and performance.

## How to Run

### Server:

```bash
cd server
make
./server
```

### Stress Test:

```bash
cd test
python3 threaded_stress_test.py --clients 100 --duration 10
```