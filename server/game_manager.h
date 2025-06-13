#pragma once

#include <chrono>
#include <string>
#include <optional>
#include "client_manager.h"
#include "../common/config.h"
enum class GameState {
    UNKNOWN,
    WAITING,
    STARTED,
    ENDED
};

/**
 * @brief Manages the game lifecycle: waiting, started, and ended phases.
 */
class GameManager {
public:
    GameManager(int max_players, int wait_time_sec);

    GameState getState() const;
    std::string getStateString() const;


  std::optional<std::string> handleMessage(
    const std::string& msg,
    const sockaddr_in& client_addr);


    void update();
    bool canAcceptClients() const;
    bool isGameRunning() const;
    void broadcastToAll(int sockfd) const {
        std::string packet = "GAME:" + getStateString() + "|" + clientManager.buildStatePacket();
        clientManager.broadcastToAll(sockfd, packet);
    }

private:
    GameState state;
    GameState lastLoggedState;
    ClientManager clientManager;
    int maxPlayers;
    int waitTimeSec;
    std::chrono::steady_clock::time_point startTime;
};
