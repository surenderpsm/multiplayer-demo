#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <chrono>
#include <atomic>

#include "../generated/game.pb.h"
#include "../common/config.h"
#define SERVER_PORT 9000
#define BUFFER_SIZE 1024

std::atomic<GameState> currentState(GameState::UNKNOWN);

void receiverThread(int sockfd, sockaddr_in& recvaddr, socklen_t& addr_len) {
    char buffer[BUFFER_SIZE];
    while (true) {
        ssize_t r = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (sockaddr*)&recvaddr, &addr_len);
        if (r > 0) {
            Packet incoming;
            if (incoming.ParseFromArray(buffer, r) && incoming.has_state_packet()) {
                const auto& sp = incoming.state_packet();
                currentState.store(sp.state());
                std::cout << "[STATE] Tick: " << sp.tick() << ", Players: " << sp.players_size() << "\n";
                for (const auto& p : sp.players()) {
                    std::cout << " - Player " << p.id() << ": (" << p.x() << ", " << p.y() << ")\n";
                }
            }
        }
    }
}

int main() {
    int sockfd;
    sockaddr_in servaddr{}, recvaddr{};
    socklen_t addr_len = sizeof(recvaddr);
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        return 1;
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

    // Send HELLO packet
    Packet hello_pkt;
    hello_pkt.mutable_hello();
    std::string data;
    hello_pkt.SerializeToString(&data);
    sendto(sockfd, data.data(), data.size(), 0, (const sockaddr*)&servaddr, sizeof(servaddr));

    // Receive WELCOME
    ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (sockaddr*)&recvaddr, &addr_len);
    Packet p;
    if (!p.ParseFromArray(buffer, n) || !p.has_welcome()) {
        std::cerr << "Unexpected or malformed welcome packet\n";
        return 1;
    }
    int client_id = p.welcome().id();
    std::cout << "[WELCOME] Assigned ID: " << client_id << "\n";

    int x = 0, y = 0;
    std::thread(receiverThread, sockfd, std::ref(recvaddr), std::ref(addr_len)).detach();

    while (true) {
        Packet outgoing;
        if (currentState.load() == GameState::STARTED) {
            auto* update = outgoing.mutable_client_update();
            update->set_id(client_id);
            update->set_x(x);
            update->set_y(y);
            x += 5;
            y += 5;
        } else {
            outgoing.mutable_ping()->set_id(client_id);
        }

        std::string out_data;
        outgoing.SerializeToString(&out_data);
        sendto(sockfd, out_data.data(), out_data.size(), 0, (const sockaddr*)&servaddr, sizeof(servaddr));
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    close(sockfd);
    return 0;
}
