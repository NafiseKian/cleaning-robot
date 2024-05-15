// NetworkModule.cpp
#include "network_module.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

NetworkModule::NetworkModule(const std::string& ip, int port)
    : serverIp(ip), serverPort(port), socketFd(-1) {}

NetworkModule::~NetworkModule() {
    if (socketFd >= 0) {
        close(socketFd);
    }
}

bool NetworkModule::connectToServer() {
    socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd < 0) {
        std::cerr << "Error creating socket." << std::endl;
        return false;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(serverPort);

    if (inet_pton(AF_INET, serverIp.c_str(), &server_addr.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported." << std::endl;
        return false;
    }

    if (connect(socketFd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection Failed." << std::endl;
        return false;
    }

    return true;
}

bool NetworkModule::sendData( std::string& data) {
    if (send(socketFd, data.c_str(), data.length(), 0) != (ssize_t)data.length()) {
        std::cerr << "Failed to send data." << std::endl;
        return false;
    }
    return true;
}
