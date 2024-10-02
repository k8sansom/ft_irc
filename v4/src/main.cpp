#include "../inc/Client.hpp"
#include "../inc/Server.hpp"
#include "../inc/lib_inc.hpp"

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    // Cleanup and close up stuff here
    exit(signum);
}

// Should we create a stricter password policy?
bool isValidPassword(const std::string& password) {
    if (password.length() < 4) {
        std::cout << "Error: Password must be at least 4 characters long." << std::endl;
        return false;
    }
    for (size_t i = 0; i < password.length(); ++i) {
        char c = password[i];
        if (!isalnum(c) && c != '!' && c != '@' && c != '#' && c != '$' && c != '%' && c != '*' && c != '&') {
            std::cout << "Error: Password contains invalid character: " << c << std::endl;
            return false;
        }
    }
    return true;
}

int main(int ac, char** av) {
    signal(SIGINT, signalHandler);  // Register signal handler
    if (ac != 3) {
        std::cout << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    try {
        int port = atoi(av[1]);
        if (port <= 0) {
            throw std::invalid_argument("Error: invalid port number");
        }

        std::string password = av[2];
        if (password.empty()) {
            std::cout << "Error: Password cannot be empty." << std::endl;
            return 1;
        }
        if (!isValidPassword(password)) {
            return 1;
        }
        Server server(port, password);
        server.setupSignalHandler();
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
