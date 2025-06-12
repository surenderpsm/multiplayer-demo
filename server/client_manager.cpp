#include "client_manager.h"
#include "utils.h"

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
