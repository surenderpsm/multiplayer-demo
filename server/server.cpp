#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <chrono>
#include <thread>

#include "game_manager.h"
#include "../common/config.h"
#include "utils.h"
#include "../generated/game.pb.h"

#define PORT 9000
#define BUFFER_SIZE 1024

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

    std::thread([&game_manager, &sockfd]() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(BROADCAST_INTERVAL_MS));
            game_manager.update();
            game_manager.broadcastToAll(sockfd);
        }
    }).detach();

    while (true) {
        socklen_t len = sizeof(client_addr);
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT,
                             (sockaddr*)&client_addr, &len);
        if (n > 0) {
            ::Packet p;
            if (!p.ParseFromArray(buffer, n)) continue;
            game_manager.handleProtobufMessage(p, client_addr, sockfd);
        }
    }

    close(sockfd);
    return 0;
}