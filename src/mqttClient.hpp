#include <memory>
#include <string>


#include "mqttMessage.hpp"
#include "tcpClient.hpp"



namespace pubsupp {

    class MqttClient {
        public:
            MqttClient(std::string& host, int port);
            ~MqttClient();

            void connect(); // get broker details from config
            void connect(std::string& brokerAddress, int brokerPort);
            void disconnect();

            void publish(const std::string& topic, QoS qos, const std::string& payload);
            void subscribe(const std::string& topic, QoS qos);

        private:
            std::unique_ptr<TcpClient> tcpClient;
            std::string host;
            int port;
            std::shared_ptr<MqttMessage> message;
            bool isConnected = false;
    };
}
