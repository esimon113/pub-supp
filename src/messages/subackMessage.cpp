#include "subackMessage.hpp"
#include <vector>
#include <stdexcept>




namespace pubsupp {
	SubackMessage::SubackMessage() {
		this->type = MessageType::SUBACK;
		this->packetId = 0;
		this->returnCode = 0;
	}


	SubackMessage::SubackMessage(uint16_t packetId, uint8_t returnCode) {
		this->type = MessageType::SUBACK;
		this->packetId = packetId;
		this->returnCode = returnCode;
	}


	std::vector<uint8_t> SubackMessage::encode() const {
		std::vector<uint8_t> buffer;

		// fixed header
		buffer.push_back(static_cast<uint8_t>(MessageType::SUBACK) << 4);

		// variable header: Packet id (2 bytes, big-endian)
		std::vector<uint8_t> variableHeader;
		variableHeader.push_back((this->packetId >> 8) & 0xFF);
		variableHeader.push_back(this->packetId & 0xFF);

		// payload: Return code (1 byte)
		variableHeader.push_back(this->returnCode);

		// remaining length (3 bytes: 2 packet id + 1 return code)
		uint32_t remainingLength = variableHeader.size();
		auto encodedRemainingLength = this->encodeRemainingLength(remainingLength);
		buffer.insert(buffer.end(), encodedRemainingLength.begin(), encodedRemainingLength.end());

		buffer.insert(buffer.end(), variableHeader.begin(), variableHeader.end());

		return buffer;
	}


	std::unique_ptr<MqttMessage> SubackMessage::decode(const std::vector<uint8_t>& data) {
		if (data.size() < 2) { throw std::runtime_error("SUBACK message too short"); }

		uint8_t fixedHeader = data[0];
		if ((fixedHeader >> 4) != static_cast<uint8_t>(MessageType::SUBACK)) {
			throw std::runtime_error("Invalid SUBACK message type");
		}

		// decode remaining length
		size_t remainingLengthStart = 1;
		size_t remainingLengthEnd = remainingLengthStart;
		while (remainingLengthEnd < data.size() && (data[remainingLengthEnd] & 128) != 0) {
			remainingLengthEnd++;
			if (remainingLengthEnd - remainingLengthStart > 4) {
				throw std::runtime_error("Malformed SUBACK: remaining length exceeds 4 bytes");
			}
		}
		if (remainingLengthEnd >= data.size()) { throw std::runtime_error("SUBACK message incomplete: missing remaining length"); }
		remainingLengthEnd++; // include the last byte

		std::vector<uint8_t> encodedLength(data.begin() + remainingLengthStart, data.begin() + remainingLengthEnd);
		uint32_t remainingLength = this->decodeRemainingLength(encodedLength);

		if (remainingLength != 3) {
			throw std::runtime_error("Invalid SUBACK remaining length: expected 3, got " + std::to_string(remainingLength));
		}

		// verify enough data is present for variable header
		size_t variableHeaderStart = remainingLengthEnd;
		if (data.size() < variableHeaderStart + 3) {
			throw std::runtime_error("SUBACK message incomplete: missing variable header");
		}

		// parse variable header: Packet id (2 bytes, big-endian)
		uint16_t packetId = (data[variableHeaderStart] << 8) | data[variableHeaderStart + 1];

		// parse payload: Return code (1 byte)
		uint8_t returnCode = data[variableHeaderStart + 2];

		return std::make_unique<SubackMessage>(packetId, returnCode);
	}


	uint16_t SubackMessage::getPacketId() const { return packetId; }
	uint8_t SubackMessage::getReturnCode() const { return returnCode; }
	bool SubackMessage::isSuccess() const { return returnCode != 0x80; }
}
