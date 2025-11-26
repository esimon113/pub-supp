#include <iostream>
#include <stdexcept>

#include "mqttClient.hpp"
#include "mqttMessage.hpp"
#include "connectMessage.hpp"
#include "connackMessage.hpp"



namespace pubsupp {

    // pass the client's configuration details
    MqttClient::MqttClient(std::string& host, int port) : host(host), port(port) {
        this->tcpClient = std::make_unique<TcpClient>(this->host, this->port);
    }


    MqttClient::~MqttClient() {
        std::cout << "Disconnecting from MQTT broker..." << std::endl;
        // Implementation to disconnect from MQTT broker
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
            std::cout << "Connection established successfully!" << std::endl;
            if (sessionPresent) { std::cout << "Session present: true" << std::endl; }

        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to receive or parse CONNACK message: " + std::string(e.what()));
        }
    }

}
