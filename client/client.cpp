#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <cstdlib>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in servaddr;

    // Create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return 1;
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

    std::cout << "Client sending to " << SERVER_IP << ":" << SERVER_PORT << "...\n";

    while (true) {
        // Generate dummy position
        int x = rand() % 100;
        int y = rand() % 100;
        std::string msg = "POS " + std::to_string(x) + "," + std::to_string(y);

        // Send position
        sendto(sockfd, msg.c_str(), msg.size(), 0, (const struct sockaddr *) &servaddr, sizeof(servaddr));
        std::cout << "[SEND] " << msg << "\n";

        // Receive echo
        socklen_t len = sizeof(servaddr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *) &servaddr, &len);
        if (n > 0) {
            buffer[n] = '\0';
            std::cout << "[ECHO] " << buffer << "\n";
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(sockfd);
    return 0;
}
