#include "publishMessage.hpp"
#include <vector>
#include <cstdint>
#include <stdexcept>




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


	static std::string decodeUTF8String(const std::vector<uint8_t>& data, size_t& offset) {
		if (offset + 2 > data.size()) {
			throw std::runtime_error("PUBLISH message incomplete: missing topic length");
		}

		uint16_t length = (data[offset] << 8) | data[offset + 1];
		offset += 2;

		if (offset + length > data.size()) {
			throw std::runtime_error("PUBLISH message incomplete: missing topic data");
		}

		std::string result(data.begin() + offset, data.begin() + offset + length);
		offset += length;

		return result;
	}


	PublishMessage::PublishMessage(const std::string& topic, QoS qos, const std::string& payload, uint16_t packetId, bool dup, bool retain)
		: topic(topic), qos(qos), payload(payload), packetId(packetId), dup(dup), retain(retain) {
		this->type = MessageType::PUBLISH;
	}


	PublishMessage::PublishMessage() {
		this->type = MessageType::PUBLISH;
		this->topic = "";
		this->qos = QoS::AT_MOST_ONCE;
		this->payload = "";
		this->packetId = 0;
		this->dup = false;
		this->retain = false;
	}


	std::vector<uint8_t> PublishMessage::encode() const {
		std::vector<uint8_t> buffer;

		// fixed header: Message type (3) << 4 | flags
		uint8_t fixedHeader = static_cast<uint8_t>(MessageType::PUBLISH) << 4;
		fixedHeader |= (this->dup ? 0x08 : 0x00); // DUP flag (bit 3)
		fixedHeader |= (static_cast<uint8_t>(this->qos) << 1); // QoS (bits 2-1)
		fixedHeader |= (this->retain ? 0x01 : 0x00); // RETAIN flag (bit 0)
		buffer.push_back(fixedHeader);

		// variable header: Topic name (UTF-8 string)
		std::vector<uint8_t> variableHeader = encodeUTF8String(this->topic);

		// Packet ID (only if QoS > 0)
		if (static_cast<uint8_t>(this->qos) > 0) {
			variableHeader.push_back((this->packetId >> 8) & 0xFF);
			variableHeader.push_back(this->packetId & 0xFF);
		}

		// remaining length
		uint32_t remainingLength = variableHeader.size() + this->payload.size();
		auto encodedRemainingLength = this->encodeRemainingLength(remainingLength);
		buffer.insert(buffer.end(), encodedRemainingLength.begin(), encodedRemainingLength.end());

		// append variable header
		buffer.insert(buffer.end(), variableHeader.begin(), variableHeader.end());

		// append payload
		for (char c : this->payload) {
			buffer.push_back(static_cast<uint8_t>(c));
		}

		return buffer;
	}


	std::unique_ptr<MqttMessage> PublishMessage::decode(const std::vector<uint8_t>& data) {
		if (data.size() < 2) {
			throw std::runtime_error("PUBLISH message too short");
		}

		uint8_t fixedHeader = data[0];
		if ((fixedHeader >> 4) != static_cast<uint8_t>(MessageType::PUBLISH)) {
			throw std::runtime_error("Invalid PUBLISH message type");
		}

		// decode flags
		bool dup = (fixedHeader & 0x08) != 0;
		QoS qos = static_cast<QoS>((fixedHeader & 0x06) >> 1);
		bool retain = (fixedHeader & 0x01) != 0;

		// decode remaining length
		size_t remainingLengthStart = 1;
		size_t remainingLengthEnd = remainingLengthStart;
		while (remainingLengthEnd < data.size() && (data[remainingLengthEnd] & 128) != 0) {
			remainingLengthEnd++;
			if (remainingLengthEnd - remainingLengthStart > 4) {
				throw std::runtime_error("Malformed PUBLISH: remaining length exceeds 4 bytes");
			}
		}
		if (remainingLengthEnd >= data.size()) {
			throw std::runtime_error("PUBLISH message incomplete: missing remaining length");
		}
		remainingLengthEnd++; // include the last byte

		std::vector<uint8_t> encodedLength(data.begin() + remainingLengthStart, data.begin() + remainingLengthEnd);
		uint32_t remainingLength = this->decodeRemainingLength(encodedLength);

		// verify enough data is present
		size_t variableHeaderStart = remainingLengthEnd;
		if (data.size() < variableHeaderStart + remainingLength) {
			throw std::runtime_error("PUBLISH message incomplete: missing data");
		}

		// decode topic name
		size_t offset = variableHeaderStart;
		std::string topic = decodeUTF8String(data, offset);

		// decode packet ID (only if QoS > 0)
		uint16_t packetId = 0;
		if (static_cast<uint8_t>(qos) > 0) {
			if (offset + 2 > data.size()) {
				throw std::runtime_error("PUBLISH message incomplete: missing packet ID");
			}
			packetId = (data[offset] << 8) | data[offset + 1];
			offset += 2;
		}

		// decode payload
		size_t payloadStart = offset;
		size_t payloadLength = remainingLength - (offset - variableHeaderStart);
		std::string payload(data.begin() + payloadStart, data.begin() + payloadStart + payloadLength);

		auto publishMsg = std::make_unique<PublishMessage>(topic, qos, payload, packetId, dup, retain);
		return publishMsg;
	}


	std::string PublishMessage::getTopic() const { return topic; }
	QoS PublishMessage::getQoS() const { return qos; }
	std::string PublishMessage::getPayload() const { return payload; }
	uint16_t PublishMessage::getPacketId() const { return packetId; }
	bool PublishMessage::isDup() const { return dup; }
	bool PublishMessage::isRetain() const { return retain; }
} // namespace pubsupp
