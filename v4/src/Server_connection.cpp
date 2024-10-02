#include "../inc/Server.hpp"

void setupSocket();
void bindSocket();
void listenSocket();
void acceptClient();
void pollClients();

std::vector<pollfd> Server::fds;
std::map<std::string, Channel> Server::channels;

void Server::setupSignalHandler() {
    signal(SIGINT, Server::signalHandler); // Set up the signal handler
}

// Signal handler (must be static, as it's a callback for the signal)
void Server::signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received. Cleaning up...\n";
    // Call the cleanup method (you can either make cleanup static or find another way to access the instance)
    cleanup();
    exit(signum);  // Exit after cleanup
}

void Server::cleanup() {
    // Close all client sockets
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        close(it->first); // Close client socket
    }
    // Close the server socket

    fds.clear();
    channels.clear();
    close(server_socket);
    std::cout << "All sockets closed. Server cleaned up." << std::endl;
}

void Server::setupSocket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("ERROR: Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
		throw std::runtime_error("ERROR: Failed to set option REUSEADDR on socket");
	}
    if (fcntl(server_socket, F_SETFL, O_NONBLOCK) == -1) {
		throw std::runtime_error("ERROR: Failed to set option NONBLOCK on socket");
	}
}

void Server::bindSocket() {
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        throw std::runtime_error("ERROR: Failed to bind socket");
    }
}

void Server::listenSocket() {
    if (listen(server_socket, 10) < 0) {
        throw std::runtime_error("ERROR: Failed to listen on socket");
    }
    std::cout << "Server listening on port " << port << std::endl;
}

void Server::acceptClient() {
    sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    int client_fd = accept(server_socket, (sockaddr*)&client_address, &client_len);
    if (client_fd < 0) {
        std::cerr << "ERROR: Failed to accept client connection" << std::endl;
        return ;
    }

    clients.insert(std::make_pair(client_fd, Client(client_fd)));
    std::cout << "New client connected: " << client_fd << std::endl;
    std::string response =
       " \r\n"                                                   
       " _|          _|  _|_|_|_|  _|          _|_|_|    _|_|    _|      _|  _|_|_|_|  \r\n"
       " _|          _|  _|        _|        _|        _|    _|  _|_|  _|_|  _|        \r\n"
       " _|    _|    _|  _|_|_|    _|        _|        _|    _|  _|  _|  _|  _|_|_|    \r\n"
       "   _|  _|  _|    _|        _|        _|        _|    _|  _|      _|  _|        \r\n"
       "     _|  _|      _|_|_|_|  _|_|_|_|    _|_|_|    _|_|    _|      _|  _|_|_|_|  \r\n"
        " \r\n"
       " \r\n"                       
       "THIS IS 3.5 SERVER\r\n"
       " \r\n";                                                                                      
    send(client_fd, response.c_str(), response.length(), 0);
}

void Server::pollClients() {
    fds.clear();

    pollfd server_fd;
    server_fd.fd = server_socket;
    server_fd.events = POLLIN;
    fds.push_back(server_fd);

    std::cout << "List of clients: " << std::endl;
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        pollfd client_fd;
        client_fd.fd = it->first;
        client_fd.events = POLLIN;
        fds.push_back(client_fd);
        std::cout << it->second.getNickname() << std::endl;
    }
    std::cout << "Number of clients: " << clients.size() << std::endl;

    int ret = poll(fds.data(), fds.size(), -1);
    if (ret < 0) {
        std::cerr << "ERROR: Poll error" << std::endl;
        return;
    } else if (ret == 0) {
        // Timeout occurred; no events happened in the last second
        return;  // Can check for exit signal or do other tasks if needed
    }

    if (fds[0].revents & POLLIN) {
        acceptClient();
    }

    for (size_t i = 1; i < fds.size(); ++i) {
        if (fds[i].revents & POLLIN) {
            handleClientMessage(fds[i].fd);
        }
    }
}
