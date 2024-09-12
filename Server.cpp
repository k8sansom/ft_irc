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

std::map<std::string, std::vector<int> >& Server::getChannels() {
    return channels;
}

void Server::setupSocket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    fcntl(server_socket, F_SETFL, O_NONBLOCK);
}

void Server::bindSocket() {
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0) {
        throw std::runtime_error("Failed to bind socket");
    }
}

void Server::listenSocket() {
    if (listen(server_socket, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }
    std::cout << "Server listening on port " << port << std::endl;
}

void Server::acceptClient() {
    sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    int client_fd = accept(server_socket, (sockaddr*)&client_address, &client_len);
    if (client_fd < 0) {
        std::cerr << "Failed to accept client connection" << std::endl;
        return;
    }

    clients.insert(std::make_pair(client_fd, Client(client_fd)));
    std::cout << "New client connected: " << client_fd << std::endl;
}

void Server::pollClients() {
    std::vector<pollfd> fds;
    
    pollfd server_fd;
    server_fd.fd = server_socket;
    server_fd.events = POLLIN;
    fds.push_back(server_fd);
    
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        pollfd client_fd;
        client_fd.fd = it->first;
        client_fd.events = POLLIN;
        fds.push_back(client_fd);
    }
    
    int ret = poll(fds.data(), fds.size(), -1);
    if (ret < 0) {
        std::cerr << "Poll error" << std::endl;
        return;
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

void Server::handleClientMessage(int client_fd) {
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes_received < 0) {
        std::cerr << "Failed to receive message from client " << client_fd << std::endl;
        return;
    } else if (bytes_received == 0) {
        close(client_fd);
        clients.erase(client_fd);
        std::cout << "Client " << client_fd << " disconnected" << std::endl;
        return;
    }

    buffer[bytes_received] = '\0';
    std::cout << "Received from client " << client_fd << ": " << buffer << std::endl;

    std::string response = "Message received\n";
    send(client_fd, response.c_str(), response.length(), 0);
}
