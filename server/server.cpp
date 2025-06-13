/* server.cpp */

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>

#include "game_manager.h"
#include "../common/config.h"
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
    GameManager game_manager(MAX_PLAYERS, WAIT_TIME_SEC);
    auto last_broadcast_time = std::chrono::steady_clock::now();

    while (true) {
        socklen_t len = sizeof(client_addr);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, MSG_DONTWAIT,
                             (sockaddr*)&client_addr, &len);
        game_manager.update();
        if (n > 0) {
            buffer[n] = '\0';
            std::string msg(buffer);

            auto response = game_manager.handleMessage(msg, client_addr);
            if (response.has_value()) {
                sendto(sockfd, response->c_str(), response->size(), 0,
                    (const sockaddr*)&client_addr, sizeof(client_addr));
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_broadcast_time).count() >= BROADCAST_INTERVAL_MS) {

            game_manager.broadcastToAll(sockfd);

            last_broadcast_time = now;
        }
    }

    close(sockfd);
    return 0;
}
