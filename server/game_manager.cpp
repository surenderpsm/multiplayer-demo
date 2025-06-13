#include "game_manager.h"
#include <iostream>

GameManager::GameManager(int max_players, int wait_time_sec)
    : maxPlayers(max_players),
      waitTimeSec(wait_time_sec) {}

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

    // PING:ID
    // This is a keep-alive message to ensure the client is still connected during waiting state.
    if (msg.rfind("PING:", 0) == 0) {
        int id;
        if (!this->clientManager.parsePingMessage(msg, id)) {
            std::cout << "[WARN] Malformed PING from " << ip_port << ": " << msg << std::endl;
            return std::nullopt; // Ignore malformed pings
        }

        // Only accept from registered clients
        if (!clientManager.validateClient(id, ip_port)) {
            std::cout << "[WARN] Rejected PING from invalid client ID: " << id << "\n";
            return std::nullopt;
        }

        clientManager.markSeen(ip_port); // <-- you’ll need to add this method
        return std::nullopt;
    }


    // UPDATE:x,y
    if (msg.rfind("UPDATE:", 0) == 0) {
        if (state != GameState::STARTED) {
            return std::nullopt; // Ignore updates if game not started
        }
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
    // --- Step 1: Prune inactive clients and count current ones ---
    clientManager.pruneInactiveClients();
    int current_players = clientManager.getClientCount();

    // --- Step 2: Handle ENDED state (no further updates allowed) ---
    if (state == GameState::ENDED) {
        if (lastLoggedState != state) {
            std::cout << "[INFO] Game has ended, no updates allowed." << std::endl;
            lastLoggedState = state;
        }
        return;
    }

    // --- Step 3: If no players, stay in WAITING and reset timer ---
    if (current_players < MIN_PLAYERS) {
        if (state != GameState::WAITING) {
            std::cout << "[INFO] Dropped below minimum players. Returning to WAITING." << std::endl;
            state = GameState::WAITING;
            lastLoggedState = state;
        }

        // Reset the wait timer ONLY if no players are online
        if (current_players == 0) {
            startTime = std::chrono::steady_clock::now();
        }

        return;
    }

    // --- Step 4: Handle transition from STARTED to ENDED if players drop mid-game ---
    if (state == GameState::STARTED && current_players < MIN_PLAYERS) {
        std::cout << "[WARN] Not enough players. Ending game." << std::endl;
        state = GameState::ENDED;
        lastLoggedState = state;
        return;
    }

    // --- Step 5: If already started, no further checks needed ---
    if (state == GameState::STARTED) {
        if (lastLoggedState != state) {
            std::cout << "[INFO] Game is already running." << std::endl;
            lastLoggedState = state;
        }
        return;
    }

    // --- Step 6: Check if we can now start the game ---
    auto now = std::chrono::steady_clock::now();
    int elapsed_sec = std::chrono::duration_cast<std::chrono::seconds>(now - startTime).count();

    if (state == GameState::WAITING) {
        if ((current_players >= maxPlayers) || (elapsed_sec >= waitTimeSec)) {
            state = GameState::STARTED;
            std::cout << "[INFO] Game has started!" << std::endl;
            lastLoggedState = state;
        }
    }
}


bool GameManager::canAcceptClients() const {
    return state == GameState::WAITING;
}

bool GameManager::isGameRunning() const {
    return state == GameState::STARTED;
}
