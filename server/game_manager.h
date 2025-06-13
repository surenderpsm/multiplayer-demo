#pragma once

#include <chrono>
#include <string>
#include <optional>
#include "client_manager.h"
#include "../common/config.h"
#include "../generated/game.pb.h"
#include <arpa/inet.h>


/**
 * @brief Manages the overall game lifecycle, including player registration,
 *        state transitions, and game state broadcasting.
 */
class GameManager {
public:
    /**
     * @brief Constructs a new GameManager instance.
     * 
     * @param max_players Maximum number of players allowed before starting
     * @param wait_time_sec Time to wait before automatically starting game
     */
    GameManager(int max_players, int wait_time_sec);

    /**
     * @brief Returns the current game state.
     */
    GameState getState() const;


    /**
     * @brief Updates the internal game state. Should be called each tick.
     * Handles timeouts, transitions, and logs state changes.
     */
    void update();

    /**
     * @brief Returns true if the server is currently accepting new clients.
     */
    bool canAcceptClients() const;

    /**
     * @brief Returns true if the game is in STARTED state.
     */
    bool isGameRunning() const;

    void broadcastToAll(int sockfd);

    void handleProtobufMessage(const Packet& packet, const sockaddr_in& client_addr, int sockfd);



private:
    GameState state;                    ///< Current state of the game
    GameState lastLoggedState;         ///< Last logged state to suppress spam logs
    ClientManager clientManager;       ///< Manages connected clients and their info
    int maxPlayers;                    ///< Max number of players allowed in game
    int waitTimeSec;                   ///< Wait duration before starting game
    std::chrono::steady_clock::time_point startTime; ///< Time when WAITING state began
    int tickCounter = 0;              ///< Counter for ticks to manage game updates
};
