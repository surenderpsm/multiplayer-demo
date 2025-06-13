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
     * @brief Parses an update message into its components.
     * 
     * Expected format: "UPDATE:<id>:<x>:<y>"
     * 
     * @param msg The raw message.
     * @param id Output parameter for extracted client ID.
     * @param x Output parameter for X position.
     * @param y Output parameter for Y position.
     * @return true if parsing succeeded, false if malformed.
     */
    bool parseUpdateMessage(const std::string& msg, int& id, int& x, int& y);
    
    /**
     * @brief Parses a ping message to extract the client ID.
     * 
     * Expected format: "PING:<id>"
     * 
     * @param msg The raw ping message.
     * @param id Output parameter for extracted client ID.
     * @return true if parsing succeeded, false if malformed.
     */
    bool parsePingMessage(const std::string& msg, int& id);

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
     * @brief Builds a state packet representing all active clients.
     * 
     * Format: "STATE:<id1>:<x1>:<y1>|<id2>:<x2>:<y2>|..."
     * 
     * @return std::string Serialized state string for broadcast.
     */
    std::string buildStatePacket() const;

    /**
     * @brief Broadcasts a message to all currently registered clients.
     * 
     * Uses each stored sockaddr_in to send via UDP.
     * 
     * @param sockfd The server socket file descriptor.
     * @param msg The message to send.
     */
    void broadcastToAll(int sockfd, const std::string& msg) const;

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


private:
    std::unordered_map<std::string, Client> clients; ///< Map from IP:Port to client struct.
    int nextClientId = 1; ///< Auto-incremented client ID generator.
};
