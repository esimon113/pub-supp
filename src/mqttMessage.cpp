#include <cstdint>
#include <vector>
#include <stdexcept>

#include "mqttMessage.hpp"
#include "connectMessage.hpp"
#include "connackMessage.hpp"




namespace pubsupp {
    std::vector<uint8_t> MqttMessage::encodeRemainingLength(uint32_t length) const {
        std::vector<uint8_t> buffer;
        uint32_t remainingLength = length;

        do {
            uint8_t byte = remainingLength % 128;
            remainingLength /= 128;

            if (remainingLength > 0) {
                byte |= 128;
            }

            buffer.push_back(byte);
        } while (remainingLength > 0);

        return buffer;
    }


    uint32_t MqttMessage::decodeRemainingLength(std::vector<uint8_t> encodedLength) const {
        uint32_t remainingLength = 0;
        uint32_t multiplier = 1;
        size_t index = 0;
        uint8_t byte;

        do {
            if (index >= encodedLength.size()) {
                throw std::runtime_error("Malformed Remaining Length: incomplete encoding");
            }

            byte = encodedLength[index++];
            remainingLength += (byte & 127) * multiplier;
            multiplier *= 128;

            if (multiplier > 128 * 128 * 128 * 128) {
                throw std::runtime_error("Malformed Remaining Length: exceeds maximum");
            }
        } while ((byte & 128) != 0);  // Continue if continuation bit is set

        return remainingLength;
    }





    // Factory function implementations
    std::unique_ptr<MqttMessage> createConnectMessage(const std::string& clientId, bool cleanSession, uint16_t keepAlive) {
        return std::make_unique<ConnectMessage>(clientId, cleanSession, keepAlive);
    }


    std::unique_ptr<MqttMessage> parseConnackMessage(const std::vector<uint8_t>& data) {
        ConnackMessage connack;
        return connack.decode(data);
    }





    // ConnackMessageHelper implementations
    bool ConnackMessageHelper::isSuccess(const MqttMessage& msg) {
        if (msg.type != MessageType::CONNACK) { throw std::runtime_error("Message is not a CONNACK message"); }
        const ConnackMessage* connack = dynamic_cast<const ConnackMessage*>(&msg);

        if (!connack) { throw std::runtime_error("Failed to cast to ConnackMessage"); }

        return connack->isSuccess();
    }


    bool ConnackMessageHelper::sessionPresent(const MqttMessage& msg) {
        if (msg.type != MessageType::CONNACK) { throw std::runtime_error("Message is not a CONNACK message"); }
        const ConnackMessage* connack = dynamic_cast<const ConnackMessage*>(&msg);

        if (!connack) { throw std::runtime_error("Failed to cast to ConnackMessage"); }

        return connack->sessionPresent();
    }


    uint8_t ConnackMessageHelper::returnCode(const MqttMessage& msg) {
        if (msg.type != MessageType::CONNACK) { throw std::runtime_error("Message is not a CONNACK message"); }
        const ConnackMessage* connack = dynamic_cast<const ConnackMessage*>(&msg);

        if (!connack) { throw std::runtime_error("Failed to cast to ConnackMessage"); }

        return connack->returnCode();
    }


    std::string ConnackMessageHelper::getReturnCodeDescription(const MqttMessage& msg) {
        if (msg.type != MessageType::CONNACK) { throw std::runtime_error("Message is not a CONNACK message"); }
        const ConnackMessage* connack = dynamic_cast<const ConnackMessage*>(&msg);

        if (!connack) { throw std::runtime_error("Failed to cast to ConnackMessage"); }

        return connack->getReturnCodeDescription();
    }
}
