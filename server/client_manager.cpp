#include "client_manager.h"
#include "utils.h"
#include "../common/config.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <cstring>
#include <cmath>

int ClientManager::registerClient(const sockaddr_in& addr) {
    std::string key = getClientKey(addr);
    if (clients.find(key) != clients.end()) {
        return clients[key].id; // Already registered
    }

    Client c;
    c.id = nextClientId++;
    c.ip_port = key;
    c.addr = addr;
    c.last_seen = std::chrono::steady_clock::now();
    clients[key] = c;

    return c.id;
}

bool ClientManager::isKnown(const std::string& ip_port) const {
    return clients.find(ip_port) != clients.end();
}

Client& ClientManager::getClient(const std::string& ip_port) {
    return clients.at(ip_port);
}

std::string ClientManager::getClientKey(const sockaddr_in& addr) const {
    return formatSockAddr(addr);
}


bool ClientManager::validateClient(int id, const std::string& ip_port) {
    auto it = clients.find(ip_port);
    return it != clients.end() && it->second.id == id;
}

void ClientManager::updateClientPosition(int id, int x, int y) {
    // @todo: replace with a more efficient search if needed
    // This is a linear search, but for small client counts it should be fine.
    for (auto& [key, client] : clients) {
        if (client.id == id) {
            client.x = x;
            client.y = y;
            client.last_seen = std::chrono::steady_clock::now();
            return;
        }
    }
}

void ClientManager::markSeen(const std::string& ip_port) {
    if (isKnown(ip_port)) {
        clients[ip_port].last_seen = std::chrono::steady_clock::now();
    }
}

void ClientManager::broadcastBinary(int sockfd, const std::string& data) const {
    for (const auto& [_, client] : clients) {
        sendto(sockfd, data.data(), data.size(), 0,
               (const struct sockaddr*)&client.addr, sizeof(client.addr));
    }
}


bool ClientManager::isCollisionFree(int x, int y, int my_id, int min_distance) const {
    for (const auto& [_, client] : clients) {
        if (client.id == my_id) continue;
        int dx = client.x - x;
        int dy = client.y - y;
        double distance = std::sqrt(dx * dx + dy * dy);
        if (distance < min_distance) return false;
    }
    return true;
}




void ClientManager::pruneInactiveClients() {
    auto now = std::chrono::steady_clock::now();
    for (auto it = clients.begin(); it != clients.end(); ) {
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - it->second.last_seen);
        if (duration.count() > CLIENT_TIMEOUT_MS) {
            std::cout << "[INFO] Dropping inactive client " << it->second.id << std::endl;
            it = clients.erase(it);
        } else {
            ++it;
        }
    }
}
