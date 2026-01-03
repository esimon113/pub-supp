#pragma once

#include "mqttMessage.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <memory>




namespace pubsupp {
    class PublishMessage : public MqttMessage {
      public:
        PublishMessage(const std::string& topic, QoS qos, const std::string& payload, uint16_t packetId = 0, bool dup = false, bool retain = false);
        PublishMessage();

        std::vector<uint8_t> encode() const override;
        std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override;

        std::string getTopic() const;
        QoS getQoS() const;
        std::string getPayload() const;
        uint16_t getPacketId() const;
        bool isDup() const;
        bool isRetain() const;


      private:
        std::string topic;
        QoS qos;
        std::string payload;
        uint16_t packetId;
        bool dup;
        bool retain;
    };
}
