#pragma once

#include <unordered_map>
#include <string>
#include <netinet/in.h>
#include "client_info.h"

class ClientManager {
public:
    /**
     * @brief Register a new client given its socket address.
     * Returns the assigned client ID.
     */
    int registerClient(const sockaddr_in& addr);

    /**
     * @brief Checks if a client is already known (by IP:port).
     */
    bool isKnown(const std::string& ip_port) const;

    /**
     * @brief Get reference to an existing client.
     */
    Client& getClient(const std::string& ip_port);

    /**
     * @brief Generates the IP:Port key from sockaddr_in.
     */
    std::string getClientKey(const sockaddr_in& addr) const;

private:
    std::unordered_map<std::string, Client> clients;
    int nextClientId = 1;
};
