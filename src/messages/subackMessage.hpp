#pragma once

#include "mqttMessage.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <memory>




namespace pubsupp {
	class SubackMessage : public MqttMessage {
		public:
			SubackMessage();
			SubackMessage(uint16_t packetId, uint8_t returnCode);

			std::vector<uint8_t> encode() const override;
			std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override;

			uint16_t getPacketId() const;
			uint8_t getReturnCode() const;
			bool isSuccess() const;


		private:
			uint16_t packetId;
			uint8_t returnCode;
	};
}

