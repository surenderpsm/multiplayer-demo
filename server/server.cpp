#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <chrono>

#include "client_manager.h"
#include "utils.h"

#define PORT 9000
#define MAXLINE 1024

int main() {
    int sockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "[START] UDP server running on port " << PORT << "\n";

    ClientManager manager;

    while (true) {
        socklen_t len = sizeof(cliaddr);
        int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }
        buffer[n] = '\0';

        std::string msg(buffer);
        std::string senderKey = manager.getClientKey(cliaddr);
        auto now = std::chrono::steady_clock::now();

        if (msg == "HELLO") {
            if (!manager.isKnown(senderKey)) {
                int assignedId = manager.registerClient(cliaddr);
                std::string response = "WELCOME:" + std::to_string(assignedId);
                sendto(sockfd, response.c_str(), response.length(), 0,
                       (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
                std::cout << "[HANDSHAKE] New client " << senderKey
                          << " assigned ID " << assignedId << "\n";
            } else {
                std::cout << "[HANDSHAKE] Duplicate HELLO from " << senderKey << "\n";
            }
            continue;
        }

        if (!manager.isKnown(senderKey)) {
            std::cout << "[DROP] Message from unknown client " << senderKey << ": " << msg << "\n";
            continue;
        }

        Client &c = manager.getClient(senderKey);
        c.last_seen = now;

        sendto(sockfd, buffer, n, 0, (const struct sockaddr *)&cliaddr, sizeof(cliaddr));
    }

    close(sockfd);
    return 0;
}
