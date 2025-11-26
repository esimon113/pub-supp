#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <memory>
#include <string>



namespace pubsupp {
    // Represented as a 4-bit unsigned value, byte 1, bits 7-4
    enum class MessageType: uint8_t {
        CONNECT = 1,
        CONNACK = 2,
        PUBLISH = 3,
        PUBACK = 4,
        PUBREC = 5,
        PUBREL = 6,
        PUBCOMP = 7,
        SUBSCRIBE = 8,
        SUBACK = 9,
        UNSUBSCRIBE = 10,
        UNSUBACK = 11,
        PINGREQ = 12,
        PINGRESP = 13,
        DISCONNECT = 14
    };


    enum class ControlFlags {
        NONE = 0x00,
        DUP = 0x08,
        QoS = 0x06, // publish QoS
        RETAIN = 0x01
    };


    enum class QoS : uint8_t {
        AT_MOST_ONCE = 0,
        AT_LEAST_ONCE = 1,
        EXACTLY_ONCE = 2
    };


    struct ConnectFlags {
        bool username = false;
        bool password = false;
        bool willRetain = false;
        QoS willQoS = QoS::AT_MOST_ONCE; // 2 bits
        bool will = false;
        bool cleanSession = false;
        // bit 0 is reserved and always has to be 0

        inline uint8_t encode() const {
            uint8_t flags = 0;
            flags |= username     ? 0b10000000 : 0;
            flags |= password     ? 0b01000000 : 0;
            flags |= willRetain   ? 0b00100000 : 0;
            flags |= static_cast<uint8_t>(willQoS) << 3;
            flags |= will         ? 0b00000100 : 0;
            flags |= cleanSession ? 0b00000010 : 0;
            return flags;
        }

        inline ConnectFlags decode(const std::vector<uint8_t>& data) {
            uint8_t flags = data[0];
            username = flags     & 0b10000000;
            password = flags     & 0b01000000;
            willRetain = flags   & 0b00100000;
            willQoS = static_cast<QoS>((flags & 0b00011000) >> 3);
            will = flags         & 0b00000100;
            cleanSession = flags & 0b00000010;
            return *this;
        }
    };


    


    class MqttMessage {
        public:
            MessageType type;
            virtual ~MqttMessage() = default;

            virtual std::vector<uint8_t> encode() const = 0;
            virtual std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) = 0;

        protected:
            // see: https://docs.oasis-open.org/mqtt/mqtt/v3.1.1/os/mqtt-v3.1.1-os.html#_Toc398718023
            std::vector<uint8_t> encodeRemainingLength(uint32_t length) const;
            uint32_t decodeRemainingLength(std::vector<uint8_t> encodedLength) const;
    };



    // factory functions for creating mqtt msgs
    std::unique_ptr<MqttMessage> createConnectMessage(const std::string& clientId = "", bool cleanSession = true, uint16_t keepAlive = 60);
    std::unique_ptr<MqttMessage> parseConnackMessage(const std::vector<uint8_t>& data);



    // helper for accessing connack properties
    class ConnackMessageHelper {
        public:
            static bool isSuccess(const MqttMessage& msg);
            static bool sessionPresent(const MqttMessage& msg);
            static uint8_t returnCode(const MqttMessage& msg);
            static std::string getReturnCodeDescription(const MqttMessage& msg);
    };

}
