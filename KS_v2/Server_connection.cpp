#include "Server.hpp"

void setupSocket();
void bindSocket();
void listenSocket();
void acceptClient();
void pollClients();


void Server::setupSocket() {
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        throw std::runtime_error("ERROR: Failed to create socket");
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
    std::string response = "WELCOME TO 3,5 SERVER\r\n";
    send(client_fd, response.c_str(), response.length(), 0);
}

void Server::pollClients() {
    std::vector<pollfd> fds;
    
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
