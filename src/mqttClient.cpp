#include <iostream>
#include <memory>
#include <stdexcept>

#include "messages/connectMessage.hpp"
#include "messages/disconnectMessage.hpp"
#include "messages/mqttMessage.hpp"
#include "messages/pubackMessage.hpp"
#include "messages/publishMessage.hpp"
#include "messages/subackMessage.hpp"
#include "messages/subscribeMessage.hpp"
#include "mqttClient.hpp"




namespace pubsupp {

	// pass the client's configuration details
	MqttClient::MqttClient(std::string& host, int port, const std::string& clientId) : host(host), port(port), clientId(clientId) {
		this->tcpClient = std::make_unique<TcpClient>(this->host, this->port);
	}


	MqttClient::~MqttClient() {
		try {
			this->disconnect();
		} catch (const std::exception& e) {
			std::cerr << "Failed to disconnect cleanly: " << e.what() << std::endl;
		}
	}


	void MqttClient::connect() { connect(this->host, this->port); }


	void MqttClient::connect(std::string& brokerAddress, int brokerPort) {
		try {
			this->tcpClient->tryConnect(brokerAddress, brokerPort);
			std::cout << "TCP connection established to " << brokerAddress << ":" << brokerPort << std::endl;

		} catch (const std::exception& e) {
			throw std::runtime_error("Failed to establish TCP connection: " + std::string(e.what()));
		}

		// create and send connect:
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
			if (sessionPresent) {
				std::cout << "Session present: true" << std::endl;
			}

		} catch (const std::exception& e) {
			throw std::runtime_error("Failed to receive or parse CONNACK message: " + std::string(e.what()));
		}
	}


	void MqttClient::disconnect() {
		if (!this->tcpClient) {
			return;
		}

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


	void MqttClient::subscribe(const std::string& topic, QoS qos, uint16_t keepalive) {
		if (!this->isConnected) {
			throw std::runtime_error("Not connected to MQTT broker");
		}

		if (static_cast<uint8_t>(qos) > 2) {
			throw std::runtime_error("Invalid QoS value: must be 0, 1, or 2");
		}

		uint16_t packetId = this->nextPacketId++;
		if (this->nextPacketId == 0) {
			this->nextPacketId = 1; // skip id 0 -> invalid
		}

		auto subscribeMsg = createSubscribeMessage(topic, qos, packetId);
		std::vector<uint8_t> subscribeData = subscribeMsg->encode();

		try {
			this->tcpClient->trySend(subscribeData);
			std::cout << "SUBSCRIBE message sent for topic: " << topic << " (QoS: " << static_cast<int>(qos) << ", packet ID: " << packetId << ")" << std::endl;
		} catch (const std::exception& e) {
			throw std::runtime_error("Failed to send SUBSCRIBE message: " + std::string(e.what()));
		}
		// receive and parse suback
		try {
			std::vector<uint8_t> subackData = this->tcpClient->tryReceiveMqttMessage();
			std::cout << "SUBACK message received (" << subackData.size() << " bytes)" << std::endl;

			auto subackMsg = parseSubackMessage(subackData);

			// Verify packet id matches
			const SubackMessage* suback = dynamic_cast<const SubackMessage*>(subackMsg.get());
			if (!suback) {
				throw std::runtime_error("Failed to cast to SubackMessage");
			}

			if (suback->getPacketId() != packetId) {
				throw std::runtime_error("SUBACK packet ID mismatch: expected " + std::to_string(packetId) + ", got " + std::to_string(suback->getPacketId()));
			}

			if (!suback->isSuccess()) {
				uint8_t returnCode = suback->getReturnCode();
				throw std::runtime_error("Subscription failed: server returned failure code (0x" + std::to_string(returnCode) + ")");
			}

			std::cout << "Subscription successful (return code: " << static_cast<int>(suback->getReturnCode()) << ")" << std::endl;

		} catch (const std::exception& e) {
			throw std::runtime_error("Failed to receive or parse SUBACK message: " + std::string(e.what()));
		}
	}


	void MqttClient::publish(const std::string& topic, QoS qos, const std::string& payload) {
		if (!this->isConnected) {
			throw std::runtime_error("Not connected to MQTT broker");
		}

		if (static_cast<uint8_t>(qos) > 2) {
			throw std::runtime_error("Invalid QoS value: must be 0, 1, or 2");
		}

		uint16_t packetId = this->nextPacketId++;
		if (this->nextPacketId == 0) {
			this->nextPacketId = 1; // skip id 0 -> invalid
		}

		auto publishMessage = createPublishMessage(topic, qos, payload, packetId);
		auto publishData = publishMessage->encode();

		try {
			this->tcpClient->trySend(publishData);
			std::cout << "PUBLISH message sent for topic: " << topic << " (QoS: " << static_cast<int>(qos) << ", packet ID: " << packetId << ")" << std::endl
					  << "\t With Payload: " << payload;
		} catch (const std::exception& e) {
			throw std::runtime_error("Failed to send PUBLISH message: " + std::string(e.what()));
		}

		if (qos == QoS::AT_LEAST_ONCE || qos == QoS::EXACTLY_ONCE) {
			// receive and parse puback
			try {
				std::vector<uint8_t> pubackData = this->tcpClient->tryReceiveMqttMessage();
				std::cout << "PUBACK message received (" << pubackData.size() << " bytes)" << std::endl;

				auto pubackMsg = parsePubackMessage(pubackData);

				// Verify packet id matches
				const PubackMessage* puback = dynamic_cast<const PubackMessage*>(pubackMsg.get());
				if (!puback) {
					throw std::runtime_error("Failed to cast to PubackMessage");
				}

				if (puback->getPacketId() != packetId) {
					throw std::runtime_error("PUBACK packet ID mismatch: expected " + std::to_string(packetId) + ", got " + std::to_string(puback->getPacketId()));
				}

				std::cout << "Publish acknowledged (packet ID: " << packetId << ")" << std::endl;

			} catch (const std::exception& e) {
				throw std::runtime_error("Failed to receive or parse PUBACK message: " + std::string(e.what()));
			}
		}
	}

} // namespace pubsupp
