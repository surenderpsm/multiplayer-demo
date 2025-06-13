#pragma once

constexpr int CLIENT_TIMEOUT_MS = 5000; // 5 seconds
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr int SERVER_PORT = 9000; // Default server port
constexpr int BROADCAST_INTERVAL_MS = 100; // Interval for broadcasting game state


// Game configuration constants
constexpr int MIN_PLAYERS = 2; // Minimum players to start the game
constexpr int MAX_PLAYERS = 10000; // Maximum players allowed in the game
constexpr int WAIT_TIME_SEC = 10; // Time to wait for players before starting the game