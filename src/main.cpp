#include <exception>
#include <iostream>
#include <string>

#include "messages/mqttMessage.hpp"
#include "mqttClient.hpp"



int main() {
	std::cout << "Connecting to MQTT broker..." << std::endl;

	try {
		std::string brokerAddress = "127.0.0.1";
		int brokerPort = 1883; // Default MQTT port
		std::string clientId = "test-client-123";

		pubsupp::MqttClient client(brokerAddress, brokerPort, clientId);

		client.connect(brokerAddress, brokerPort);

		std::string topic = "#";
		pubsupp::QoS qos = pubsupp::QoS::AT_MOST_ONCE;
		uint16_t keepalive = 0;

		std::cout << "Subscribing to topic: " << topic << std::endl;
		client.subscribe(topic, qos, keepalive);

		std::cout << "Successfully subscribed to topic!" << std::endl;

		std::string payload = "Hello MQTT";
		std::string publishTopic = "test";
		std::cout << "Publishing " << payload.length() << " bytes to topic: " << publishTopic << std::endl;
		client.publish(publishTopic, qos, payload);
		std::cout << "Successfully published to topic!" << std::endl;

		// TODO: implement message handling -> what should happen if client receives a message?
		std::cout << "Press Enter to disconnect..." << std::endl;
		std::cin.get();

		client.disconnect();
		std::cout << "Disconnected from MQTT broker." << std::endl;

	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}

	return 0;
}
