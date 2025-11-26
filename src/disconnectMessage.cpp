#include <stdexcept>

#include "disconnectMessage.hpp"




namespace pubsupp {

    DisconnectMessage::DisconnectMessage() {
        this->type = MessageType::DISCONNECT;
    }


    std::vector<uint8_t> DisconnectMessage::encode() const {
        std::vector<uint8_t> buffer;
        
        buffer.push_back(static_cast<uint8_t>(MessageType::DISCONNECT) << 4);
        buffer.push_back(0x00); // remaining length is 0

        return buffer;
    }


    std::unique_ptr<MqttMessage> DisconnectMessage::decode(const std::vector<uint8_t>& data) {
        if (data.size() < 2) {
            throw std::runtime_error("DISCONNECT message too short");
        }

        uint8_t fixedHeader = data[0];
        if ((fixedHeader >> 4) != static_cast<uint8_t>(MessageType::DISCONNECT)) {
            throw std::runtime_error("Invalid DISCONNECT message type");
        }

        if (data[1] != 0x00) {
            throw std::runtime_error("Invalid DISCONNECT remaining length");
        }

        return std::make_unique<DisconnectMessage>();
    }

}

