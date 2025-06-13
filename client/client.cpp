#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000

int main() {
    int sockfd;
    sockaddr_in servaddr{}, recvaddr{};
    socklen_t addr_len = sizeof(recvaddr);
    char buffer[1024];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return 1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

    // Step 1: Send HELLO
    std::string hello = "HELLO";
    sendto(sockfd, hello.c_str(), hello.size(), 0, (const sockaddr*)&servaddr, sizeof(servaddr));

    // Step 2: Wait for WELCOME response
    ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0, (sockaddr*)&recvaddr, &addr_len);
    if (n <= 0) {
        std::cerr << "No response from server\n";
        return 1;
    }
    buffer[n] = '\0';
    std::string response(buffer);
    std::cout << "[RECV] " << response << "\n";

    int client_id = -1;
    if (response.rfind("WELCOME:", 0) == 0) {
        client_id = std::stoi(response.substr(8));
    } else {
        std::cerr << "Unexpected handshake response\n";
        return 1;
    }

    // Step 3: Periodically send updates
    int x = 0, y = 0;
    while (true) {
        std::string update = "UPDATE:" + std::to_string(client_id) + ":" + std::to_string(x) + ":" + std::to_string(y);
        sendto(sockfd, update.c_str(), update.size(), 0, (const sockaddr*)&servaddr, sizeof(servaddr));

        // Listen for state packet
        ssize_t r = recvfrom(sockfd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT, (sockaddr*)&recvaddr, &addr_len);
        if (r > 0) {
            buffer[r] = '\0';
            std::cout << "[STATE] " << buffer << "\n";
        }

        x += 5;
        y += 5;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(sockfd);
    return 0;
}
