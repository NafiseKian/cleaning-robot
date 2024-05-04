#include <iostream>
#include <boost/asio.hpp>
#include "https_module.h"

using namespace boost::asio;

HTTPS::HTTPS() : service(), ep(ip::address::from_string("127.0.0.1"), 2001), sock(service) {
    sock.connect(ep);
    std::cout << "Connected to server." << std::endl;
}

int HTTPS::WriteToSocket(std::string dataToSend) {
    try {
        // Send GPS data to server
        write(sock, buffer(dataToSend));

        // Wait for response from server (if any)
        char data[1024];
        size_t length = sock.read_some(buffer(data));

        std::cout << "Response from server: " << std::string(data, length) << std::endl;
        
        return 0;
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << std::endl;
        return 1;
    }
}
