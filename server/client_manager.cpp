#include "client_manager.h"
#include "utils.h"
#include "../common/config.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <unistd.h>
#include <chrono>
#include <cstring>

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

bool ClientManager::parsePingMessage(const std::string& msg, int& id) {
    // Format: PING:<id>
    std::istringstream stream(msg);
    std::string token;

    if (!std::getline(stream, token, ':') || token != "PING") return false;
    if (!std::getline(stream, token, ':')) return false;
    id = std::stoi(token);

    return true;
}

bool ClientManager::parseUpdateMessage(const std::string& msg, int& id, int& x, int& y) {
    // Format: UPDATE:<id>:<x>:<y>
    std::istringstream stream(msg);
    std::string token;

    if (!std::getline(stream, token, ':') || token != "UPDATE") return false;
    if (!std::getline(stream, token, ':')) return false;
    id = std::stoi(token);
    if (!std::getline(stream, token, ':')) return false;
    x = std::stoi(token);
    if (!std::getline(stream, token)) return false;
    y = std::stoi(token);

    return true;
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

std::string ClientManager::buildStatePacket() const {
    std::ostringstream packet;
    packet << "STATE:";

    bool first = true;
    for (const auto& [_, client] : clients) {
        if (!first) packet << "|";
        packet << client.id << ":" << client.x << ":" << client.y;
        first = false;
    }

    return packet.str();
}

void ClientManager::broadcastToAll(int sockfd, const std::string& msg) const {
    for (const auto& [_, client] : clients) {
        sendto(sockfd, msg.c_str(), msg.size(), 0,
               (const struct sockaddr*)&client.addr, sizeof(client.addr));
    }
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
