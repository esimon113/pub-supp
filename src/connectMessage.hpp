#pragma once

#include "mqttMessage.hpp"
#include <string>
#include <cstdint>
#include <vector>
#include <memory>





namespace pubsupp {

    class ConnectMessage : public MqttMessage {
        public:
            ConnectMessage(const std::string& clientId = "", bool cleanSession = true, uint16_t keepAlive = 60);
            
            std::vector<uint8_t> encode() const override;

            // not necessary for connect message:
            std::unique_ptr<MqttMessage> decode(const std::vector<uint8_t>& data) override { return nullptr; };


        private:
            std::string clientId;
            bool cleanSession;
            uint16_t keepAlive;
            ConnectFlags flags;
    };

}


