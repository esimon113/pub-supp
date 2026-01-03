#include <memory>
#include <string>


#include "messages/mqttMessage.hpp"
#include "tcpClient.hpp"



namespace pubsupp {

	class MqttClient {
	  public:
		MqttClient(std::string& host, int port, const std::string& clientId);
		~MqttClient();

		void connect(); // get broker details from config
		void connect(std::string& brokerAddress, int brokerPort);
		void disconnect();

		void publish(const std::string& topic, QoS qos, const std::string& payload);
		void subscribe(const std::string& topic, QoS qos, uint16_t keepalive);

	  private:
		std::unique_ptr<TcpClient> tcpClient;
		std::string host;
		int port;
		const std::string& clientId;
		std::shared_ptr<MqttMessage> message;
		bool isConnected = false;
		uint16_t nextPacketId = 1;
	};
} // namespace pubsupp
