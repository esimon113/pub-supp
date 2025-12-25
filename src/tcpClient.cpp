#include <cstring>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "tcpClient.hpp"

namespace pubsupp {
void TcpClient::initializeSocket() {
// note: windows specific code is AI-generated, I only implemented and tested it
// on Linux...
#ifdef _WIN32
  if (!this->socketInitialized) {
	WSADATA wsaData;

	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0) {
	  throw std::runtime_error("WSAStartup failed: " + std::to_string(result));
	}

	this->socketInitialized = true;
  }
#endif
}

void TcpClient::cleanupSocket() {
#ifdef _WIN32
  if (this->socketInitialized) {
	WSACleanup();

	this->socketInitialized = false;
  }
#endif
}

TcpClient::TcpClient()
	: ipAddress("127.0.0.1"), port(8080), tcpSocket(INVALID_SOCKET_VALUE),
	  socketInitialized(false) {
  std::cout << "TcpClient created with IP " << ipAddress << " and port " << port
			<< std::endl;
  this->initializeSocket();

  this->tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (tcpSocket == INVALID_SOCKET_VALUE) {
	this->cleanupSocket();

	throw std::runtime_error("Failed to create socket");
  }
}

TcpClient::TcpClient(std::string &serverAddress, int serverPort)
	: ipAddress(serverAddress), port(serverPort),
	  tcpSocket(INVALID_SOCKET_VALUE), socketInitialized(false) {
  std::cout << "TcpClient created with IP " << ipAddress << " and port " << port
			<< std::endl;
  this->initializeSocket();

  this->tcpSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (tcpSocket == INVALID_SOCKET_VALUE) {
	this->cleanupSocket();

	throw std::runtime_error("Failed to create socket");
  }
}

TcpClient::~TcpClient() {
  std::cout << "TcpClient destroyed" << std::endl;
  this->disconnect();
  this->cleanupSocket();
}

void TcpClient::tryConnect(std::string &serverAddress, int serverPort) {
  this->serverAddress = serverAddress;
  this->serverPort = serverPort;

  struct sockaddr_in server;
  std::memset(&server, 0, sizeof(server));
  server.sin_family = AF_INET;
  server.sin_port = htons(this->serverPort);

#ifdef _WIN32
  // For windows: inet_pton is available in ws2tcpip.h...
  if (inet_pton(AF_INET, this->serverAddress.c_str(), &server.sin_addr) <= 0) {
	server.sin_addr.s_addr = inet_addr(this->serverAddress.c_str());

	if (server.sin_addr.s_addr == INADDR_NONE ||
		server.sin_addr.s_addr == INADDR_ANY) {
	  throw std::runtime_error("Invalid IP address: " + this->serverAddress);
	}
  }
#else
  // Linux/macOS: inet_pton is available in arpa/inet.h
  if (inet_pton(AF_INET, this->serverAddress.c_str(), &server.sin_addr) <= 0) {
	server.sin_addr.s_addr = inet_addr(this->serverAddress.c_str());

	if (server.sin_addr.s_addr == INADDR_NONE) {
	  throw std::runtime_error("Invalid IP address: " + this->serverAddress);
	}
  }
#endif

  if (::connect(this->tcpSocket, (struct sockaddr *)&server, sizeof(server)) ==
	  SOCKET_ERROR_VALUE) {
#ifdef _WIN32
	int error = WSAGetLastError();
	throw std::runtime_error("Failed to connect to server " + serverAddress +
							 ":" + std::to_string(this->serverPort) +
							 " (Error: " + std::to_string(error) + ")");
#else
	throw std::runtime_error("Failed to connect to server " + serverAddress +
							 ":" + std::to_string(this->serverPort));
#endif
  }
}

void TcpClient::trySend(std::string &message) {
#ifdef _WIN32
  int bytesSent = ::send(this->tcpSocket, message.c_str(),
						 static_cast<int>(message.size()), 0);
#else
  ssize_t bytesSent =
	  ::send(this->tcpSocket, message.c_str(), message.size(), 0);
#endif
  if (bytesSent == SOCKET_ERROR_VALUE) {
	throw std::runtime_error("Failed to send message");
  }
}

void TcpClient::trySend(const std::vector<uint8_t> &data) {
#ifdef _WIN32
  int bytesSent =
	  ::send(this->tcpSocket, reinterpret_cast<const char *>(data.data()),
			 static_cast<int>(data.size()), 0);
#else
  ssize_t bytesSent = ::send(this->tcpSocket, data.data(), data.size(), 0);
#endif
  if (bytesSent == SOCKET_ERROR_VALUE) {
	throw std::runtime_error("Failed to send binary data");
  }
}

std::string TcpClient::tryReceive(int bufferSize) {
  std::vector<char> buffer(bufferSize);
#ifdef _WIN32
  int bytesRead = ::recv(this->tcpSocket, buffer.data(), bufferSize - 1, 0);
#else
  ssize_t bytesRead = ::recv(this->tcpSocket, buffer.data(), bufferSize - 1, 0);
#endif

  if (bytesRead == SOCKET_ERROR_VALUE) {
	throw std::runtime_error("Failed to receive message");
  }

  buffer[bytesRead] = '\0';
  return std::string(buffer.data());
}

std::vector<uint8_t> TcpClient::tryReceiveBinary(size_t bufferSize) {
  std::vector<uint8_t> buffer(bufferSize);
#ifdef _WIN32
  int bytesRead =
	  ::recv(this->tcpSocket, reinterpret_cast<char *>(buffer.data()),
			 static_cast<int>(bufferSize), 0);
#else
  ssize_t bytesRead = ::recv(this->tcpSocket, buffer.data(), bufferSize, 0);
#endif

  // TODO: custom error handling
  if (bytesRead == SOCKET_ERROR_VALUE) {
	throw std::runtime_error("Failed to receive binary data");
  }
  if (bytesRead == 0) {
	throw std::runtime_error("Connection closed by peer");
  }

  buffer.resize(bytesRead);
  return buffer;
}

std::vector<uint8_t> TcpClient::tryReceiveMqttMessage() {
  // fixed header: 1 byte
  std::vector<uint8_t> fixedHeader = this->tryReceiveBinary(1);
  if (fixedHeader.empty()) {
	throw std::runtime_error("Failed to receive MQTT fixed header");
  }

  // remaining length: 1-4 bytes
  std::vector<uint8_t> remainingLengthBytes;
  uint8_t byte;
  uint32_t remainingLength = 0;
  uint32_t multiplier = 1;
  int lengthBytesRead = 0;

  do {
	std::vector<uint8_t> lengthByte = this->tryReceiveBinary(1);
	if (lengthByte.empty()) {
	  throw std::runtime_error("Failed to receive remaining length byte");
	}

	byte = lengthByte[0];
	remainingLengthBytes.push_back(byte);
	remainingLength += (byte & 127) * multiplier;
	multiplier *= 128;
	lengthBytesRead++;

	if (lengthBytesRead > 4) {
	  throw std::runtime_error(
		  "Malformed MQTT message: remaining length exceeds 4 bytes");
	}
  } while ((byte & 128) != 0);

  // rest of message
  std::vector<uint8_t> messageBody = this->tryReceiveBinary(remainingLength);

  // put it all together.
  std::vector<uint8_t> fullMessage;
  fullMessage.push_back(fixedHeader[0]);
  fullMessage.insert(fullMessage.end(), remainingLengthBytes.begin(),
					 remainingLengthBytes.end());
  fullMessage.insert(fullMessage.end(), messageBody.begin(), messageBody.end());

  return fullMessage;
}

void TcpClient::disconnect() {
  if (this->tcpSocket == INVALID_SOCKET_VALUE) {
	return;
  }
#ifdef _WIN32
  if (::closesocket(this->tcpSocket) == SOCKET_ERROR) {
	throw std::runtime_error("Failed to close socket");
  }
#else
  if (::close(this->tcpSocket) == -1) {
	throw std::runtime_error("Failed to close socket");
  }
#endif
  this->tcpSocket = INVALID_SOCKET_VALUE;
}

} // namespace pubsupp
