#include "game_manager.h"
#include <iostream>

GameManager::GameManager(int max_players, int wait_time_sec)
    : state(GameState::WAITING),
      maxPlayers(max_players),
      waitTimeSec(wait_time_sec),
      startTime(std::chrono::steady_clock::now()) {}

GameState GameManager::getState() const {
    return state;
}

std::string GameManager::getStateString() const {
    switch (state) {
        case GameState::WAITING: return "WAITING";
        case GameState::STARTED: return "STARTED";
        case GameState::ENDED: return "ENDED";
        default: return "UNKNOWN";
    }
}


std::optional<std::string> GameManager::handleMessage(
    const std::string& msg,
    const sockaddr_in& client_addr) {
    std::string ip_port = this->clientManager.getClientKey(client_addr);    
    // HELLO handshake
    if (msg == "HELLO") {
        if (!canAcceptClients()) {
            std::cout << "[REJECT] Late HELLO from " << ip_port << std::endl;
            return "REJECT:GAME_ALREADY_STARTED";
        }

        if (!this->clientManager.isKnown(ip_port)) {
            int id = this->clientManager.registerClient(client_addr);
            std::cout << "[HANDSHAKE] New client " << ip_port << " assigned ID " << id << std::endl;
            return "WELCOME:" + std::to_string(id);
        } else {
            std::cout << "[HANDSHAKE] Duplicate HELLO from " << ip_port << std::endl;
            return {}; // no response for duplicates
        }
    }

    // UPDATE:x,y
    if (msg.rfind("UPDATE:", 0) == 0) {
        int id, x, y;
        if (this->clientManager.parseUpdateMessage(msg, id, x, y)) {
            if (this->clientManager.validateClient(id, ip_port)) {
                this->clientManager.updateClientPosition(id, x, y);
                std::cout << "[UPDATE] Client " << id << " at (" << x << ", " << y << ")" << std::endl;
            } else {
                std::cout << "[DROP] ID mismatch or unknown client " << ip_port << std::endl;
            }
        } else {
            std::cout << "[WARN] Malformed UPDATE: " << msg << std::endl;
        }
        return {}; // No reply to UPDATEs
    }

    std::cout << "[RECV] Unknown message from " << ip_port << ": " << msg << std::endl;
    return {};
}


void GameManager::update() {
    int current_players = this->clientManager.getClientCount();
    if (current_players < 1) {
        std::cout << "[WARN] No players connected, cannot start game." << std::endl;
        return;
    }
    if (state == GameState::ENDED) {
        std::cout << "[INFO] Game has ended, no updates allowed." << std::endl;
        return;
    }
    if (state == GameState::STARTED) {
        std::cout << "[INFO] Game is already running." << std::endl;
        return;
    }
    auto now = std::chrono::steady_clock::now();
    int elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    if (state == GameState::WAITING) {
        if (current_players >= maxPlayers || elapsed_sec >= waitTimeSec) {
            state = GameState::STARTED;
        }
    }
}

bool GameManager::canAcceptClients() const {
    return state == GameState::WAITING;
}

bool GameManager::isGameRunning() const {
    return state == GameState::STARTED;
}
