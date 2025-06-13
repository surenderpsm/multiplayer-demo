#ifndef GAME_MANAGER_H
#define GAME_MANAGER_H

#include "client_manager.h"
#include "../generated/game.pb.h"
#include <chrono>

class GameManager {
public:
    GameManager(int max_players, int wait_time_sec);

    /**
     * Check if the game is currently in the STARTED state.
     * @return true if game is running, false otherwise.
     */
    bool isGameRunning() const;

    /**
     * Determine if new clients can still join the game.
     * @return true if game is in WAITING state and accepting clients.
     */
    bool canAcceptClients() const;

    /**
     * Advance the game tick counter and remove inactive clients.
     */
    void update();

    /**
     * Broadcast the full game state to all connected clients.
     * Uses a StatePacket inside a Protobuf Packet.
     * @param sockfd UDP socket descriptor used to send data.
     */
    void broadcastToAll(int sockfd);

    /**
     * Handle a Protobuf message received from a client.
     * Performs registration, ping processing, and client updates.
     * @param packet Parsed Protobuf packet.
     * @param client_addr The address of the client.
     * @param sockfd Socket used for sending responses.
     */
    void handleProtobufMessage(const Packet& packet, const sockaddr_in& client_addr, int sockfd);

private:
    GameState state;               ///< Current game state (WAITING, STARTED, etc.)
    int tickCounter;               ///< Game tick count
    int maxPlayers;               ///< Max allowed players
    int waitTimeSec;              ///< Seconds to wait before game auto-starts
    std::chrono::steady_clock::time_point startTime;
    GameState lastLoggedState = GameState::UNKNOWN; ///< Last logged state for info messages

    ClientManager clientManager;  ///< Tracks all client states and metadata
};

#endif // GAME_MANAGER_H