#include <iostream>

#include "mqttClient.hpp"



namespace pubsupp {

    // pass the client's configuration details
    MqttClient::MqttClient(std::string& host, int port) : _host(host), _port(port) {
        this->_tcpClient = std::make_unique<TcpClient>(this->_host, this->_port);
    }

    MqttClient::~MqttClient() {
        std::cout << "Disconnecting from MQTT broker..." << std::endl;
        // Implementation to disconnect from MQTT broker
    }

    void MqttClient::connect() {
        // Implementation to connect to MQTT broker using configuration details
    }

    void MqttClient::connect(std::string& brokerAddress, int brokerPort) {

        // establish a tcp-connection
        try {
            this->_tcpClient->tryConnect(brokerAddress, brokerPort);
        } catch (const std::exception& e) {
            throw std::runtime_error("Failed to connect to MQTT broker: " + std::string(e.what()));
        }
    }

}
