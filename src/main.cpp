#include <exception>
#include <iostream>
#include <string>

#include "mqttClient.hpp"



int main() {
    std::cout << "Connecting to MQTT broker..." << std::endl;

    try {
        std::string brokerAddress = "127.0.0.1";
        int brokerPort = 1883;  // Default MQTT port

        pubsupp::MqttClient client(brokerAddress, brokerPort);
        client.connect(brokerAddress, brokerPort);

        std::cout << "Successfully connected to MQTT broker!" << std::endl;

        // TODO: Add publish/subscribe functionality here

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
