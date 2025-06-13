#pragma once

#include <string>
#include <chrono>
#include <netinet/in.h>  // for sockaddr_in

/**
 * @brief Represents a single connected client in the multiplayer system.
 *
 * The server uses this struct to store connection-specific data such as
 * assigned ID, IP address and port, current position, and last activity timestamp.
 * This enables tracking client sessions and maintaining synchronized game state.
 */
struct Client {
    /**
     * @brief Unique ID assigned by the server to identify the client.
     *
     * This integer ID is used in protocol messages and state broadcasting.
     */
    int id;

    /**
     * @brief IP and port string key (used for identification and map lookup).
     *
     * Format: "127.0.0.1:54321"
     */
    std::string ip_port;

    /**
     * @brief Raw socket address of the client, used for sending messages back.
     *
     * Required for use in `sendto()`. This is set when the server first receives a packet.
     */
    sockaddr_in addr;

    /**
     * @brief Most recent position update from the client.
     */
    int x = 0;
    int y = 0;

    bool blocked = false;  ///< Indicates if the client is currently blocked (e.g., due to collision)

    /**
     * @brief Timestamp of the last received packet from this client.
     *
     * Used to detect inactive or disconnected clients based on elapsed time.
     */
    std::chrono::steady_clock::time_point last_seen;
};
