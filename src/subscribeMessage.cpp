#include "subscribeMessage.hpp"
#include <vector>
#include <cstdint>




namespace pubsupp {
	static std::vector<uint8_t> encodeUTF8String(const std::string& str) {
		std::vector<uint8_t> result;
		uint16_t length = static_cast<uint16_t>(str.length());

		// big-endian
		result.push_back((length >> 8) & 0xFF);
		result.push_back(length & 0xFF);

		for (char c : str) {
			result.push_back(static_cast<uint8_t>(c));
		}

		return result;
	}



	SubscribeMessage::SubscribeMessage(const std::string& topic, QoS qos, uint16_t packetId)
		: topic(topic), qos(qos), packetId(packetId) {
		this->type = MessageType::SUBSCRIBE;
	}


	std::vector<uint8_t> SubscribeMessage::encode() const {
		std::vector<uint8_t> buffer;

		// fixed header: Message type (8) << 4 | reserved bits (0x02)
		buffer.push_back((static_cast<uint8_t>(MessageType::SUBSCRIBE) << 4) | 0x02);

		// variable header: Packet identifier (2 bytes, big-endian)
		std::vector<uint8_t> variableHeader;
		variableHeader.push_back((this->packetId >> 8) & 0xFF);
		variableHeader.push_back(this->packetId & 0xFF);

		// payload: Topic filter + QoS (1 byte)
		std::vector<uint8_t> topicEncoded = encodeUTF8String(this->topic);
		variableHeader.insert(variableHeader.end(), topicEncoded.begin(), topicEncoded.end());
		variableHeader.push_back(static_cast<uint8_t>(this->qos));

		// remaining length
		uint32_t remainingLength = variableHeader.size();
		auto encodedRemainingLength = this->encodeRemainingLength(remainingLength);
		buffer.insert(buffer.end(), encodedRemainingLength.begin(), encodedRemainingLength.end());

		buffer.insert(buffer.end(), variableHeader.begin(), variableHeader.end());

		return buffer;
	}
}
