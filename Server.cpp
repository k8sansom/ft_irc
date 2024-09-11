#include "Server.hpp"

Server::Server(int port, const std::string& password): port(port), password(password) {
    setupSocket();
    bindSocket();
    listenSocket();
}

Server::~Server() {}


std::map<int, Client>& Server::getClients() {
    return clients;
}
std::map<std::string, std::vector<int>>& Server::getChannels() {
    return channels;
}

void Server::setupSocket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0); // 0 for TCP by default
    if (server_socket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1; // enables the following function
    // allows the server to reuse the address (port) immediately after the server is stopped or restarted.
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 

    // Set the socket to non-blocking
    fcntl(server_socket, F_SETFL, O_NONBLOCK);
}

void Server::bindSocket() {
    server_address.sin_family = AF_INET; // address will be IPv4
    server_address.sin_addr.s_addr = INADDR_ANY; //  binds the socket to all available network interfaces on the machine
    server_address.sin_port = htons(port); // specifies the port, htons converts the port number from host byte order to network byte order 

    // associates the socket with the specified address (INADDR_ANY) and port
    // (sockaddr*)&server_address: a pointer to the sockaddr_in structure that contains the IP address and port information
    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }
}

void Server::listenSocket() {

    // listen -- marks the socket as a passive socket (accepting all incoming conection requests)
    // 10 is a maximum incoming connection rewuests in the queue before serv starts rejecting new requests
    if (listen(server_socket, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }
    std::cout << "Server listening on port " << port << std::endl;
}

bool Server::checkName(std::string nickname) {
    return true;
}