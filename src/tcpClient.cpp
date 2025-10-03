#include <iostream>
#include <stdexcept>
#include <unistd.h>

#include "tcpClient.hpp"


namespace pubsupp {
    TcpClient::TcpClient()
        : _ipAddress("127.0.0.1"),
        _port(8080),
        _socket(-1)
    {
        std::cout << "TcpClient created with IP " << _ipAddress << " and port " << _port << std::endl;

        this->_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket == -1) {
            throw std::runtime_error("Failed to create socket"); // TODO: custom error management
        }

    }


    TcpClient::TcpClient(std::string& serverAddress, int serverPort)
        : _ipAddress(serverAddress),
          _port(serverPort),
          _socket(-1)
    {
        std::cout << "TcpClient created with IP " << _ipAddress << " and port " << _port << std::endl;

        this->_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (_socket == -1) {
            throw std::runtime_error("Failed to create socket"); // TODO: custom error management
        }
    }

    TcpClient::~TcpClient() {
        std::cout << "TcpClient destroyed" << std::endl;
    }


    void TcpClient::tryConnect(std::string& serverAddress, int serverPort) {
        this->_serverAddress = serverAddress;
        this->_port = serverPort;

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(_port);
        server.sin_addr.s_addr = inet_addr(_ipAddress.c_str());

        if (::connect(_socket, (struct sockaddr*)&server, sizeof(server)) == -1) {
            throw std::runtime_error("Failed to connect to server");
        }
    }


    void TcpClient::trySend(std::string& message) {
        if (::send(_socket, message.c_str(), message.size(), 0) == -1) {
            throw std::runtime_error("Failed to send message");
        }
    }


    std::string TcpClient::tryReceive(int bufferSize) {
        char buffer[bufferSize];
        ssize_t bytesRead = ::recv(_socket, buffer, bufferSize - 1, 0);

        if (bytesRead == -1) {
            throw std::runtime_error("Failed to receive message");
        }

        buffer[bytesRead] = '\0';

        return std::string(buffer);
    }


    void TcpClient::disconnect() {
        if (this->_socket == -1)
            return;

        if (::close(this->_socket) == -1) {
            throw std::runtime_error("Failed to close socket");
        }
    }


}
