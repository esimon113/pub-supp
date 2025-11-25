#pragma once

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <vector>



namespace pubsupp {

    class TcpClient {
        public:
            TcpClient();
            TcpClient(std::string& serverAddress, int serverPort);
            ~TcpClient();

            void tryConnect(std::string& serverAddress, int serverPort);
            void disconnect();
            void trySend(std::string& message);
            void trySend(const std::vector<uint8_t>& data);
            std::string tryReceive(int bufferSize);
            std::vector<uint8_t> tryReceiveBinary(size_t bufferSize);
            // Try reading MQTT msg with proper length handling:
            std::vector<uint8_t> tryReceiveMqttMessage();

        private:
            std::string ipAddress;
            int port;
            int tcpSocket;
            std::string serverAddress;
            int serverPort;
    };

}
