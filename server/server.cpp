/* server.cpp */

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>

#include "client_manager.h"
#include "utils.h"

#define PORT 9000
#define BUFFER_SIZE 1024
#define BROADCAST_INTERVAL_MS 100

int main() {
    int sockfd;
    sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    std::cout << "[START] UDP server running on port " << PORT << std::endl;

    ClientManager manager;
    auto last_broadcast_time = std::chrono::steady_clock::now();

    while (true) {
        socklen_t len = sizeof(client_addr);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, MSG_DONTWAIT,
                             (sockaddr*)&client_addr, &len);

        if (n > 0) {
            buffer[n] = '\0';
            std::string msg(buffer);
            std::string ip_port = manager.getClientKey(client_addr);

            if (msg == "HELLO") {
                if (!manager.isKnown(ip_port)) {
                    int new_id = manager.registerClient(client_addr);
                    std::string welcome = "WELCOME:" + std::to_string(new_id);
                    sendto(sockfd, welcome.c_str(), welcome.size(), 0,
                           (const sockaddr*)&client_addr, sizeof(client_addr));
                    std::cout << "[HANDSHAKE] New client " << ip_port << " assigned ID " << new_id << std::endl;
                } else {
                    std::cout << "[HANDSHAKE] Duplicate HELLO from " << ip_port << std::endl;
                }
            } else if (msg.rfind("UPDATE:", 0) == 0) {
                int id, x, y;
                if (manager.parseUpdateMessage(msg, id, x, y)) {
                    if (manager.validateClient(id, ip_port)) {
                        manager.updateClientPosition(id, x, y);
                        std::cout << "[UPDATE] Client " << id << " at (" << x << ", " << y << ")" << std::endl;
                    } else {
                        std::cout << "[DROP] ID mismatch or unknown client " << ip_port << std::endl;
                    }
                } else {
                    std::cout << "[WARN] Malformed UPDATE message: " << msg << std::endl;
                }
            } else {
                std::cout << "[RECV] Unknown message from " << ip_port << ": " << msg << std::endl;
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_broadcast_time).count() >= BROADCAST_INTERVAL_MS) {
            std::string state_packet = manager.buildStatePacket();
            manager.broadcastToAll(sockfd, state_packet);
            last_broadcast_time = now;
        }
    }

    close(sockfd);
    return 0;
}
