#pragma once

#include <string>
#include <vector>



// set platform-specific includes:
#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")
	typedef SOCKET SocketType;
	#define INVALID_SOCKET_VALUE INVALID_SOCKET
	#define SOCKET_ERROR_VALUE SOCKET_ERROR
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <sys/types.h>
	typedef int SocketType;
	#define INVALID_SOCKET_VALUE -1
	#define SOCKET_ERROR_VALUE -1
#endif





namespace pubsupp {

	class TcpClient {
		public:
			TcpClient();
			TcpClient(std::string& serverAddress, int serverPort);
			~TcpClient();

			void tryConnect(std::string& serverAddress, int serverPort);
			void disconnect();
			void trySend(std::string& message);
			void trySend(const std::vector<uint8_t>& data);
			std::string tryReceive(int bufferSize);
			std::vector<uint8_t> tryReceiveBinary(size_t bufferSize);
			// Try reading MQTT msg with proper length handling:
			std::vector<uint8_t> tryReceiveMqttMessage();

		private:
			void initializeSocket();
			void cleanupSocket();

			std::string ipAddress;
			int port;
			SocketType tcpSocket;
			std::string serverAddress;
			int serverPort;
			bool socketInitialized;
	};

}
