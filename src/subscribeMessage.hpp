#pragma once

#include "mqttMessage.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <memory>




namespace pubsupp {

	class SubscribeMessage : public MqttMessage {
		public:
			SubscribeMessage(const std::string& topic, QoS qos, uint16_t packetId);

			std::vector<uint8_t> encode() const override;

			// not necessary for subscribe message:
			std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override { return nullptr; };


		private:
			std::string topic;
			QoS qos;
			uint16_t packetId;
	};

}

