#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>



namespace pubsupp {

    class TcpClient {
        public:
            TcpClient();
            ~TcpClient();

            void tryConnect(std::string& serverAddress, int serverPort);
            void disconnect();
            void trySend(std::string& message);
            std::string tryReceive(int bufferSize);

        private:
            std::string _ipAddress;
            int _port;
            int _socket;
            std::string _serverAddress;
            int _serverPort;
    };

}
