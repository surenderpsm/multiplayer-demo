#pragma once

#include <unordered_map>
#include <string>
#include <netinet/in.h>
#include "client_info.h"

/**
 * @brief Manages all connected clients for the multiplayer server.
 * 
 * Tracks client registrations, validates incoming updates, and builds
 * state packets for broadcasting. Clients are uniquely identified by
 * their IP:Port string and assigned a numeric ID on registration.
 */
class ClientManager {
public:
    /**
     * @brief Registers a new client given its socket address.
     * 
     * Generates a unique client ID and stores its metadata for tracking.
     * 
     * @param addr Socket address of the incoming client.
     * @return int Assigned unique client ID.
     */
    int registerClient(const sockaddr_in& addr);

    /**
     * @brief Checks if a client is already known based on IP:Port.
     * 
     * @param ip_port The IP:Port string identifier.
     * @return true if the client is already registered.
     */
    bool isKnown(const std::string& ip_port) const;

    /**
     * @brief Retrieves the client struct associated with a given IP:Port.
     * 
     * @param ip_port The IP:Port string identifier.
     * @return Client& Reference to the stored client.
     */
    Client& getClient(const std::string& ip_port);

    /**
     * @brief Formats a sockaddr_in as an IP:Port string.
     * 
     * @param addr The socket address.
     * @return std::string IP:Port formatted string.
     */
    std::string getClientKey(const sockaddr_in& addr) const;

    /**
     * @brief Validates whether an incoming update is from a known client.
     * 
     * Ensures that the client ID matches the stored IP:Port.
     * 
     * @param id Claimed client ID in the message.
     * @param ip_port Source IP:Port string of the packet.
     * @return true if the client is valid and registered.
     */
    bool validateClient(int id, const std::string& ip_port);

    /**
     * @brief Marks a client as seen by updating its last seen timestamp.
     * 
     * This is used to refresh the client's activity status.
     * 
     * @param ip_port The IP:Port string identifier of the client.
     */
    void markSeen(const std::string& ip_port);

    /**
     * @brief Updates the position of a registered client.
     * 
     * Also updates the timestamp of the last received message.
     * 
     * @param id Registered client ID.
     * @param x New X coordinate.
     * @param y New Y coordinate.
     */
    void updateClientPosition(int id, int x, int y);


    /**
     * @brief Gets the total number of registered clients.
     * 
     * @return int Count of currently registered clients.
     */
    int getClientCount() const {
        return clients.size();
    }

    /**
     * @brief Removes clients that have not sent updates within the timeout period.
     * 
     * Cleans up inactive clients to free resources and maintain accurate state.
     */
    void pruneInactiveClients();

    /**
     * Broadcast a Protobuf packet to all registered clients.
     * @param sockfd UDP socket to send with.
     * @param data Serialized packet to send.
     */
    void broadcastBinary(int sockfd, const std::string& data) const;

    /**
     * Get a read-only reference to the map of connected clients.
     * @return Map of client ID to ClientInfo.
     */
    const std::unordered_map<std::string, Client>& getClients() const { return clients; }
    
    /**
     * Check if any client is within the given radius of the (x,y) position.
     * @param x X coordinate.
     * @param y Y coordinate.
     * @param radius Distance threshold.
     * @return true if a collision is detected, false otherwise.
     */
    bool isCollisionFree(int x, int y, int my_id, int min_distance = 1) const;

private:
    std::unordered_map<std::string, Client> clients; ///< Map from IP:Port to client struct.
    int nextClientId = 1; ///< Auto-incremented client ID generator.
};
