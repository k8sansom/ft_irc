#include "Client.hpp"
#include "Server.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

int main(int ac, char** av) {
    if (ac != 3) {
        std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    try {
        int port = std::stoi(av[1]);
        if (port <= 0) {
            throw std::invalid_argument("Error: invalid port number");
        }

        std::string password = av[2];
        if (password.empty()) {
            std::cout << "Error: Password cannot be empty." << std::endl;
            return 1;
        }

        Server server(port, password);
        std::cout << "Starting server on port " << av[1] << " with password: " << av[2] << std::endl;

        while (true) {
            server.pollClients();
        }
    }
    catch (const std::invalid_argument& e) {
        std::cout << "Error: Invalid port number." << std::endl;
        return 1;
    }

    return 0;
}
