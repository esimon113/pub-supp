#pragma once

#include "mqttMessage.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <memory>





namespace pubsupp {
	class ConnackMessage : public MqttMessage {
		public:
			ConnackMessage();
			ConnackMessage(bool sessionPresent, uint8_t returnCode);

			std::vector<uint8_t> encode() const override;
			std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override;

			bool sessionPresent() const;
			uint8_t returnCode() const;
			bool isSuccess() const;
			std::string getReturnCodeDescription() const;


		private:
			bool sessionPresentFlag;
			uint8_t returnCodeValue;
	};
}
