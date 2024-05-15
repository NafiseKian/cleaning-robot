// NetworkModule.h
#ifndef NETWORK_MODULE_H
#define NETWORK_MODULE_H

#include <string>

class NetworkModule {
public:
    NetworkModule(const std::string& serverIp, int serverPort);
    ~NetworkModule();

    bool connectToServer();
    bool sendData(const std::string& data);

private:
    std::string serverIp;
    int serverPort;
    int socketFd; 
};

#endif
