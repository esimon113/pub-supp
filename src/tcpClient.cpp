#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <vector>

#include "tcpClient.hpp"


namespace pubsupp {
    TcpClient::TcpClient()
        : ipAddress("127.0.0.1"),
        port(8080),
        tcpSocket(-1)
    {
        std::cout << "TcpClient created with IP " << ipAddress << " and port " << port << std::endl;

        this->tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (tcpSocket == -1) { throw std::runtime_error("Failed to create socket"); } // TODO: custom error management
    }


    TcpClient::TcpClient(std::string& serverAddress, int serverPort)
        : ipAddress(serverAddress),
          port(serverPort),
          tcpSocket(-1)
    {
        std::cout << "TcpClient created with IP " << ipAddress << " and port " << port << std::endl;

        this->tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (tcpSocket == -1) { throw std::runtime_error("Failed to create socket"); } // TODO: custom error management
    }


    TcpClient::~TcpClient() {
        std::cout << "TcpClient destroyed" << std::endl;
    }


    void TcpClient::tryConnect(std::string& serverAddress, int serverPort) {
        this->serverAddress = serverAddress;
        this->port = serverPort;

        struct sockaddr_in server;
        server.sin_family = AF_INET;
        server.sin_port = htons(serverPort);
        server.sin_addr.s_addr = inet_addr(serverAddress.c_str());

        if (server.sin_addr.s_addr == INADDR_NONE) {
            throw std::runtime_error("Invalid IP address: " + serverAddress);
        }

        if (::connect(tcpSocket, (struct sockaddr*)&server, sizeof(server)) == -1) {
            throw std::runtime_error("Failed to connect to server");
        }
    }


    void TcpClient::trySend(std::string& message) {
        if (::send(tcpSocket, message.c_str(), message.size(), 0) == -1) {
            throw std::runtime_error("Failed to send message");
        }
    }


    void TcpClient::trySend(const std::vector<uint8_t>& data) {
        if (::send(tcpSocket, data.data(), data.size(), 0) == -1) {
            throw std::runtime_error("Failed to send binary data");
        }
    }


    std::string TcpClient::tryReceive(int bufferSize) {
        char buffer[bufferSize];
        ssize_t bytesRead = ::recv(tcpSocket, buffer, bufferSize - 1, 0);

        if (bytesRead == -1) { throw std::runtime_error("Failed to receive message"); }

        buffer[bytesRead] = '\0';

        return std::string(buffer);
    }


    std::vector<uint8_t> TcpClient::tryReceiveBinary(size_t bufferSize) {
        std::vector<uint8_t> buffer(bufferSize);
        ssize_t bytesRead = ::recv(tcpSocket, buffer.data(), bufferSize, 0);

        // TODO: custom error management:
        if (bytesRead == -1) { throw std::runtime_error("Failed to receive binary data"); }
        if (bytesRead == 0) { throw std::runtime_error("Connection closed by peer"); }

        buffer.resize(bytesRead);
        return buffer;
    }


    std::vector<uint8_t> TcpClient::tryReceiveMqttMessage() {
        // fixed header: 1 byte
        std::vector<uint8_t> fixedHeader = tryReceiveBinary(1);
        if (fixedHeader.empty()) { throw std::runtime_error("Failed to receive MQTT fixed header"); }

        // remaining length: 1-4 bytes
        std::vector<uint8_t> remainingLengthBytes;
        uint8_t byte;
        uint32_t remainingLength = 0;
        uint32_t multiplier = 1;
        int lengthBytesRead = 0;

        do {
            std::vector<uint8_t> lengthByte = tryReceiveBinary(1);
            if (lengthByte.empty()) { throw std::runtime_error("Failed to receive remaining length byte"); }

            byte = lengthByte[0];
            remainingLengthBytes.push_back(byte);
            remainingLength += (byte & 127) * multiplier;
            multiplier *= 128;
            lengthBytesRead++;

            if (lengthBytesRead > 4) { throw std::runtime_error("Malformed MQTT message: remaining length exceeds 4 bytes"); }
        } while ((byte & 128) != 0);

        // rest of message
        std::vector<uint8_t> messageBody = tryReceiveBinary(remainingLength);

        // put it all together.
        std::vector<uint8_t> fullMessage;
        fullMessage.push_back(fixedHeader[0]);
        fullMessage.insert(fullMessage.end(), remainingLengthBytes.begin(), remainingLengthBytes.end());
        fullMessage.insert(fullMessage.end(), messageBody.begin(), messageBody.end());

        return fullMessage;
    }


    void TcpClient::disconnect() {
        if (this->tcpSocket == -1) { return; }
        if (::close(this->tcpSocket) == -1) { throw std::runtime_error("Failed to close socket"); }
    }


}
