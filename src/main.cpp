#include <exception>
#include <iostream>
#include <string>

#include "tcpClient.hpp"



int main() {
    std::cout << "Hello World!" << std::endl;



    try {
        std::string msg = "Hello Server!";
        std::string srvAddr = "127.0.0.1";

        auto client = pubsupp::TcpClient();
        client.tryConnect(srvAddr, 8080);
        client.trySend(msg);
        std::cout << client.tryReceive(1024) << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }


    return 0;
}
