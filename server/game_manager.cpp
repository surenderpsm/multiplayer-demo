#include "game_manager.h"
#include <iostream>
#include <arpa/inet.h>
#include "../common/config.h"


using GameState = ::GameState;

GameManager::GameManager(int max_players, int wait_time_sec)
    : maxPlayers(max_players),
      waitTimeSec(wait_time_sec) {}

void GameManager::handleProtobufMessage(const Packet& packet, const sockaddr_in& client_addr, int sockfd) {
    std::string ip_port = clientManager.getClientKey(client_addr);

    if (packet.has_hello()) {
        if (!canAcceptClients()) {
            std::cout << "[REJECT] Late HELLO from " << ip_port << std::endl;
            return;
        }

        if (!clientManager.isKnown(ip_port)) {
            int id = clientManager.registerClient(client_addr);
            std::cout << "[HANDSHAKE] Registered client " << ip_port << " -> ID " << id << std::endl;

            Packet reply;
            reply.mutable_welcome()->set_id(id);

            std::string binary;
            reply.SerializeToString(&binary);
            sendto(sockfd, binary.data(), binary.size(), 0, (sockaddr*)&client_addr, sizeof(client_addr));
        }

    } else if (packet.has_ping()) {
        int id = packet.ping().id();

        if (!clientManager.validateClient(id, ip_port)) {
            std::cout << "[WARN] Invalid PING from ID=" << id << " at " << ip_port << std::endl;
            return;
        }

        clientManager.markSeen(ip_port);

    } else if (packet.has_client_update()) {
        if (state != GameState::STARTED) return;

        const auto& update = packet.client_update();
        int id = update.id();
        int x = update.x();
        int y = update.y();

        if (clientManager.validateClient(id, ip_port)) {
            if (clientManager.isCollisionFree(x, y, id, 50)) {
                clientManager.updateClientPosition(id, x, y);
                clientManager.setBlocked(id, false);
                std::cout << "[UPDATE] ID=" << id << " → (" << x << "," << y << ")\n";
            } else {
                clientManager.setBlocked(id, true);
                std::cout << "[BLOCKED] ID=" << id << " attempted to move too close to another player\n";
            }
        } else {
            std::cout << "[DROP] Mismatched update from " << ip_port << std::endl;
        }

    } else {
        std::cout << "[WARN] Unknown or empty Packet from " << ip_port << std::endl;
    }
}



void GameManager::update() {
    tickCounter++;
    for (auto& [_, client] : clientManager.getClientsMutable()) {
        client.blocked = false;
    }
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
            tickCounter = 0; // Reset tick counter on start
            std::cout << "[INFO] Game has started!" << std::endl;
            lastLoggedState = state;
        }
    }
}


void GameManager::broadcastToAll(int sockfd) {
    Packet wrapper;
    StatePacket* sp = wrapper.mutable_state_packet();
    sp->set_state(static_cast<::GameState>(state));
    sp->set_tick(tickCounter);

    for (const auto& [_, client] : clientManager.getClients()) {
        Player* p = sp->add_players();
        p->set_id(client.id);
        p->set_x(client.x);
        p->set_y(client.y);
        p->set_blocked(client.blocked);
    }


    std::string binary;
    wrapper.SerializeToString(&binary);
    clientManager.broadcastBinary(sockfd, binary);

    // Send state packet to local viewer GUI
    sockaddr_in gui_addr{};
    gui_addr.sin_family = AF_INET;
    gui_addr.sin_port = htons(9999); // Must match Python GUI's UDP_PORT
    inet_pton(AF_INET, "127.0.0.1", &gui_addr.sin_addr);

    sendto(sockfd, binary.data(), binary.size(), 0,
        (sockaddr*)&gui_addr, sizeof(gui_addr));

}

bool GameManager::canAcceptClients() const {
    return state == GameState::WAITING;
}

bool GameManager::isGameRunning() const {
    return state == GameState::STARTED;
}
