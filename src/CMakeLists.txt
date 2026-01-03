cmake_minimum_required(VERSION 3.30)

project(pubsupp)

set(CMAKE_CXX_STANDARD 20)

add_executable(pubsupp
	main.cpp
	tcpClient.cpp
	mqttClient.cpp
	topic.cpp
	messages/mqttMessage.cpp
	messages/connectMessage.cpp
	messages/connackMessage.cpp
	messages/disconnectMessage.cpp
	messages/subscribeMessage.cpp
	messages/subackMessage.cpp
	messages/publishMessage.cpp
	messages/pubackMessage.cpp
)
