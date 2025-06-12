#pragma once

#include <string>
#include <netinet/in.h>  // for sockaddr_in
#include <arpa/inet.h>   // for inet_ntop
#include <cstring>       // for memset

/**
 * @brief Converts a sockaddr_in structure into a human-readable "IP:Port" string.
 *
 * This function is typically used to identify clients by their IP and port combination.
 * 
 * @param addr The sockaddr_in structure to convert.
 * @return A string in the format "xxx.xxx.xxx.xxx:port".
 */
inline std::string formatSockAddr(const sockaddr_in& addr) {
    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
    int port = ntohs(addr.sin_port);
    return std::string(ip) + ":" + std::to_string(port);
}