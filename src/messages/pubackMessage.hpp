
#pragma once

#include "mqttMessage.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>




namespace pubsupp {
	class PubackMessage : public MqttMessage {
	  public:
		PubackMessage();
		PubackMessage(uint16_t packetId);

		std::vector<uint8_t> encode() const override;
		std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override;

		uint16_t getPacketId() const;


	  private:
		uint16_t packetId;
	};
} // namespace pubsupp
