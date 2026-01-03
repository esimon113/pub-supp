#include "connackMessage.hpp"
#include <vector>
#include <stdexcept>





namespace pubsupp {
	ConnackMessage::ConnackMessage() {
		this->type = MessageType::CONNACK;
		this->sessionPresentFlag = false;
		this->returnCodeValue = 0;
	}


	ConnackMessage::ConnackMessage(bool sessionPresent, uint8_t returnCode) {
		this->type = MessageType::CONNACK;
		this->sessionPresentFlag = sessionPresent;
		this->returnCodeValue = returnCode;
	}


	std::vector<uint8_t> ConnackMessage::encode() const {
		std::vector<uint8_t> buffer;

		// fixed header
		buffer.push_back(static_cast<uint8_t>(MessageType::CONNACK) << 4);

		// remaining length
		buffer.push_back(0x02);

		// variable header
		buffer.push_back(this->sessionPresentFlag ? 0x01 : 0x00);

		// connect return code
		buffer.push_back(this->returnCodeValue);

		return buffer;
	}


	std::unique_ptr<MqttMessage> ConnackMessage::decode(const std::vector<uint8_t>& data) {
		if (data.size() < 2) { throw std::runtime_error("CONNACK message too short"); }

		// verify fixed header
		uint8_t fixedHeader = data[0];
		if ((fixedHeader >> 4) != static_cast<uint8_t>(MessageType::CONNACK)) {
			throw std::runtime_error("Invalid CONNACK message type");
		}

		// decode remaining length
		size_t remainingLengthStart = 1;
		size_t remainingLengthEnd = remainingLengthStart;
		while (remainingLengthEnd < data.size() && (data[remainingLengthEnd] & 128) != 0) {
			remainingLengthEnd++;
			if (remainingLengthEnd - remainingLengthStart > 4) {
				throw std::runtime_error("Malformed CONNACK: remaining length exceeds 4 bytes");
			}
		}
		if (remainingLengthEnd >= data.size()) { throw std::runtime_error("CONNACK message incomplete: missing remaining length"); }
		remainingLengthEnd++; // include the last byte

		std::vector<uint8_t> encodedLength(data.begin() + remainingLengthStart, data.begin() + remainingLengthEnd);
		uint32_t remainingLength = this->decodeRemainingLength(encodedLength);

		if (remainingLength != 2) {
			throw std::runtime_error("Invalid CONNACK remaining length: expected 2, got " + std::to_string(remainingLength));
		}

		// verify enough data is present for variable header
		size_t variableHeaderStart = remainingLengthEnd;
		if (data.size() < variableHeaderStart + 2) {
			throw std::runtime_error("CONNACK message incomplete: missing variable header");
		}

		// parse variable header
		bool sessionPresent = (data[variableHeaderStart] & 0x01) != 0;
		uint8_t returnCode = data[variableHeaderStart + 1];

		return std::make_unique<ConnackMessage>(sessionPresent, returnCode);
	}


	bool ConnackMessage::sessionPresent() const { return sessionPresentFlag; }
	uint8_t ConnackMessage::returnCode() const { return returnCodeValue; }
	bool ConnackMessage::isSuccess() const { return returnCodeValue == 0; }


	std::string ConnackMessage::getReturnCodeDescription() const {
		switch (returnCodeValue) {
			case 0: return "Connection Accepted";
			case 1: return "Connection Refused: unacceptable protocol version";
			case 2: return "Connection Refused: identifier rejected";
			case 3: return "Connection Refused: server unavailable";
			case 4: return "Connection Refused: bad user name or password";
			case 5: return "Connection Refused: not authorized";
			default: return "Connection Refused: unknown error";
		}
	}
}
