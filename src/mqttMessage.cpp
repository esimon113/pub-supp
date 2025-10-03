

#include <algorithm>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

#include "mqttMessage.hpp"




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

        do {
            uint8_t byte = encodedLength.front();
            encodedLength.erase(encodedLength.begin());

            remainingLength += (byte & 127) * multiplier;
            multiplier *= 128;

            if (multiplier > 128 * 128 * 128) {
                throw std::runtime_error("Malformed Remaining Length");
            }
        } while (encodedLength.front() & 128);

        return remainingLength;
    }



    class ConnectMessage : public MqttMessage {
        public:
            ConnectMessage() {
                this->type = MessageType::CONNECT;
                this->_clientId = "";
                this->_cleanSession = true;
                this->_keepAlive = 60;
                this->_flags = ConnectFlags();
            }

            std::vector<uint8_t> encode() const override {
                std::vector<uint8_t> buffer;

                // message type
                buffer.push_back((uint8_t)type << 4);
                // remaining length
                auto remainingLength = this->encodeRemainingLength(10);
                std::copy(remainingLength.begin(), remainingLength.end(), std::back_inserter(buffer));

                ConnectFlags flags { // TODO: make configurable
                    .username = false,
                    .password = false,
                    .willRetain = false,
                    .willQoS = QoS::AT_MOST_ONCE,
                    .will = false,
                    .cleanSession = true
                };

                // TODO: use variable header struct and encode it
                ConnectVariableHeader variableHeader {
                    .connectFlags = flags.encode(),
                    .keepAlive = this->_keepAlive
                };
                auto encodedHeader = variableHeader.encode();
                std::copy(encodedHeader.begin(), encodedHeader.end(), std::back_inserter(buffer));

                return buffer;
            }

            std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override {


                return std::make_unique<ConnectMessage>();
            }

        private:
            std::string _clientId;
            bool _cleanSession;
            uint16_t _keepAlive;
            ConnectFlags _flags;
    };




    class ConnackMessage : public MqttMessage {
        public:
            ConnackMessage();

            std::vector<uint8_t> encode() const override;
            std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override;


        private:
            bool _sessionPresent;
            uint8_t _returnCode;
    };
}
