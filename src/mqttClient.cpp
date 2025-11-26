#include <iostream>
#include <stdexcept>

#include "mqttClient.hpp"
#include "mqttMessage.hpp"
#include "connectMessage.hpp"
#include "connackMessage.hpp"
#include "disconnectMessage.hpp"





namespace pubsupp {

    // pass the client's configuration details
    MqttClient::MqttClient(std::string& host, int port) : host(host), port(port) {
        this->tcpClient = std::make_unique<TcpClient>(this->host, this->port);
    }


    MqttClient::~MqttClient() {
        try {
            this->disconnect();
        } catch (const std::exception& e) {
            std::cerr << "Failed to disconnect cleanly: " << e.what() << std::endl;
        }
    }


    void MqttClient::connect() {
        connect(this->host, this->port);
    }


    void MqttClient::connect(std::string& brokerAddress, int brokerPort) {
        try {
            this->tcpClient->tryConnect(brokerAddress, brokerPort);
            std::cout << "TCP connection established to " << brokerAddress << ":" << brokerPort << std::endl;

        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to establish TCP connection: " + std::string(e.what()));
        }

        // create and send connect:
        std::string clientId = "pubsupp_client"; // TODO: make configurable
        auto connectMsg = createConnectMessage(clientId, true, 60);
        std::vector<uint8_t> connectData = connectMsg->encode();

        try {
            this->tcpClient->trySend(connectData);
            std::cout << "CONNECT message sent (" << connectData.size() << " bytes)" << std::endl;

        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to send CONNECT message: " + std::string(e.what()));
        }

        // receive and parse connack:
        try {
            std::vector<uint8_t> connackData = this->tcpClient->tryReceiveMqttMessage();
            std::cout << "CONNACK message received (" << connackData.size() << " bytes)" << std::endl;

            auto connackMsg = parseConnackMessage(connackData);

            if (!ConnackMessageHelper::isSuccess(*connackMsg)) {
                uint8_t returnCode = ConnackMessageHelper::returnCode(*connackMsg);
                std::string description = ConnackMessageHelper::getReturnCodeDescription(*connackMsg);

                throw std::runtime_error("Connection refused: " + description + " (code: " + std::to_string(returnCode) + ")");
            }

            bool sessionPresent = ConnackMessageHelper::sessionPresent(*connackMsg);
            this->isConnected = true;
            std::cout << "Connection established successfully!" << std::endl;
            if (sessionPresent) { std::cout << "Session present: true" << std::endl; }

        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to receive or parse CONNACK message: " + std::string(e.what()));
        }
    }

    
    void MqttClient::disconnect() {
        if (!this->tcpClient) { return; }

        // create + send disconnect packet
        if (this->isConnected) {
            try {
                DisconnectMessage disconnectMsg;
                auto payload = disconnectMsg.encode();
                this->tcpClient->trySend(payload);
                std::cout << "DISCONNECT message sent (" << payload.size() << " bytes)" << std::endl;

            } catch (const std::exception& e) {
                throw std::runtime_error("Failed to send DISCONNECT message: " + std::string(e.what()));
            }
        }

        // disconnect from tcp socket
        try {
            this->tcpClient->disconnect();
            this->isConnected = false;
            std::cout << "TCP connection closed" << std::endl;

        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to disconnect from MQTT broker: " + std::string(e.what()));
        }
    }

}
