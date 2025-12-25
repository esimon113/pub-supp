#include <exception>
#include <iostream>
#include <string>

#include "mqttClient.hpp"
#include "mqttMessage.hpp"



int main() {
	std::cout << "Connecting to MQTT broker..." << std::endl;

	try {
		std::string brokerAddress = "127.0.0.1";
		int brokerPort = 1883;  // Default MQTT port

		pubsupp::MqttClient client(brokerAddress, brokerPort, clientId);


		client.connect(brokerAddress, brokerPort);

		std::string topic = "test";
		pubsupp::QoS qos =  pubsupp::QoS::AT_MOST_ONCE;
		uint16_t keepalive = 0;
		client.subscribe(topic, qos, keepalive);


		std::cout << "Successfully connected to MQTT broker!" << std::endl;

		// TODO: Add publish/subscribe functionality here

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
