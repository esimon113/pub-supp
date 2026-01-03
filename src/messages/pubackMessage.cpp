
#include "pubackMessage.hpp"
#include <stdexcept>
#include <vector>




namespace pubsupp {
	PubackMessage::PubackMessage() {
		this->type = MessageType::PUBACK;
		this->packetId = 0;
	}


	PubackMessage::PubackMessage(uint16_t packetId) {
		this->type = MessageType::PUBACK;
		this->packetId = packetId;
	}


	std::vector<uint8_t> PubackMessage::encode() const {
		std::vector<uint8_t> buffer;

		// fixed header
		buffer.push_back(static_cast<uint8_t>(MessageType::PUBACK) << 4);

		// variable header: Packet id (2 bytes, big-endian)
		std::vector<uint8_t> variableHeader;
		variableHeader.push_back((this->packetId >> 8) & 0xFF);
		variableHeader.push_back(this->packetId & 0xFF);

		// remaining length (2 bytes: packet id)
		uint32_t remainingLength = variableHeader.size();
		auto encodedRemainingLength = this->encodeRemainingLength(remainingLength);
		buffer.insert(buffer.end(), encodedRemainingLength.begin(), encodedRemainingLength.end());

		buffer.insert(buffer.end(), variableHeader.begin(), variableHeader.end());

		return buffer;
	}


	std::unique_ptr<MqttMessage> PubackMessage::decode(const std::vector<uint8_t>& data) {
		if (data.size() < 2) {
			throw std::runtime_error("PUBACK message too short");
		}

		uint8_t fixedHeader = data[0];
		if ((fixedHeader >> 4) != static_cast<uint8_t>(MessageType::PUBACK)) {
			throw std::runtime_error("Invalid PUBACK message type");
		}

		// decode remaining length
		size_t remainingLengthStart = 1;
		size_t remainingLengthEnd = remainingLengthStart;
		while (remainingLengthEnd < data.size() && (data[remainingLengthEnd] & 128) != 0) {
			remainingLengthEnd++;
			if (remainingLengthEnd - remainingLengthStart > 4) {
				throw std::runtime_error("Malformed PUBACK: remaining length exceeds 4 bytes");
			}
		}
		if (remainingLengthEnd >= data.size()) {
			throw std::runtime_error("PUBACK message incomplete: missing remaining length");
		}
		remainingLengthEnd++; // include the last byte

		std::vector<uint8_t> encodedLength(data.begin() + remainingLengthStart, data.begin() + remainingLengthEnd);
		uint32_t remainingLength = this->decodeRemainingLength(encodedLength);

		if (remainingLength != 2) {
			throw std::runtime_error("Invalid PUBACK remaining length: expected 2, got " + std::to_string(remainingLength));
		}

		// verify enough data is present for variable header
		size_t variableHeaderStart = remainingLengthEnd;
		if (data.size() < variableHeaderStart + 2) {
			throw std::runtime_error("PUBACK message incomplete: missing variable header");
		}

		// parse variable header: Packet id (2 bytes, big-endian)
		uint16_t packetId = (data[variableHeaderStart] << 8) | data[variableHeaderStart + 1];

		return std::make_unique<PubackMessage>(packetId);
	}


	uint16_t PubackMessage::getPacketId() const { return packetId; }
} // namespace pubsupp
