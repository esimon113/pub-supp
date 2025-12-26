cmake_minimum_required(VERSION 3.30)

project(pubsupp)

set(CMAKE_CXX_STANDARD 20)

add_executable(pubsupp
	main.cpp
	tcpClient.cpp
	mqttClient.cpp
	topic.cpp
	mqttMessage.cpp
	connectMessage.cpp
	connackMessage.cpp
	disconnectMessage.cpp
)
