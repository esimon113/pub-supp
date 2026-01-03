#pragma once

#include <vector>
#include <memory>

#include "mqttMessage.hpp"




namespace pubsupp {

	class DisconnectMessage : public MqttMessage {
		public:
			DisconnectMessage();

			std::vector<uint8_t> encode() const override;
			std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override;
	};

}
