#include "connectMessage.hpp"
#include <vector>
#include <cstdint>
#include <stdexcept>





namespace pubsupp {
    static std::vector<uint8_t> encodeUTF8String(const std::string& str) {
        std::vector<uint8_t> result;
        uint16_t length = static_cast<uint16_t>(str.length());

        // big-endian:
        result.push_back((length >> 8) & 0xFF);
        result.push_back(length & 0xFF);

        for (char c : str) { result.push_back(static_cast<uint8_t>(c)); }

        return result;
    }





    ConnectMessage::ConnectMessage(const std::string& clientId, bool cleanSession, uint16_t keepAlive)
        : clientId(clientId), cleanSession(cleanSession), keepAlive(keepAlive) {
        this->type = MessageType::CONNECT;
        this->flags = ConnectFlags();
        this->flags.cleanSession = cleanSession;
    }


    std::vector<uint8_t> ConnectMessage::encode() const {
        std::vector<uint8_t> buffer;

        // fixed header
        buffer.push_back(static_cast<uint8_t>(MessageType::CONNECT) << 4);

        // variable header
        std::vector<uint8_t> variableHeader;

        // protocol name
        variableHeader.push_back(0x00);
        variableHeader.push_back(0x04);
        variableHeader.push_back('M');
        variableHeader.push_back('Q');
        variableHeader.push_back('T');
        variableHeader.push_back('T');

        // protocol versoin: 4 (= MQTT 3.1.1)
        variableHeader.push_back(0x04);

        // connect flags
        ConnectFlags flags;
        flags.cleanSession = this->cleanSession;
        flags.will = false;
        flags.willQoS = QoS::AT_MOST_ONCE;
        flags.willRetain = false;
        flags.username = false;
        flags.password = false;
        variableHeader.push_back(flags.encode());

        // keep alive
        variableHeader.push_back((this->keepAlive >> 8) & 0xFF);
        variableHeader.push_back(this->keepAlive & 0xFF);

        // payload
        std::vector<uint8_t> clientIdEncoded = encodeUTF8String(this->clientId);
        variableHeader.insert(variableHeader.end(), clientIdEncoded.begin(), clientIdEncoded.end());

        // remaining length
        uint32_t remainingLength = variableHeader.size();
        auto encodedRemainingLength = this->encodeRemainingLength(remainingLength);
        buffer.insert(buffer.end(), encodedRemainingLength.begin(), encodedRemainingLength.end());

        // append variable header and payload
        buffer.insert(buffer.end(), variableHeader.begin(), variableHeader.end());

        return buffer;
    }
}

