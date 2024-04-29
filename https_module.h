#ifndef HTTPS_MODULE_H
#define HTTPS_MODULE_H

#include <string>
#include <iostream>
#include <boost/asio.hpp>

class HTTPS {
public:
    HTTPS();
    int WriteToSocket(std::string data);
private:
    boost::asio::io_service service;
    boost::asio::ip::tcp::endpoint ep;
    boost::asio::ip::tcp::socket sock;
};

#endif
