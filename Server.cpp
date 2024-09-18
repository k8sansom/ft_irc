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
    
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        pollfd client_fd;
        client_fd.fd = it->first;
        client_fd.events = POLLIN;
        fds.push_back(client_fd);
        std::cout << it->second.getNickname() << ":" << std::endl;
    }
    std::cout << "Number of clients: " << clients.size() << std::endl; 
    
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
    if (clients.find(client_fd) == clients.end()) {
        std::cerr << "Client not found in the list" << std::endl;
        return;
    }

    std::vector<std::string> messages = receiveMessage(client_fd);
    if (messages.empty()) 
        return ;

    for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); ++it) {
        std::string message = *it;
        if (message.empty()) 
            continue;

        std::cout << "Received from client " << client_fd << ": " << message << std::endl;

        if (!clients[client_fd].isAuthenticated() && message.rfind("PASS", 0) != 0) {
            std::string auth_response = "ERROR :You need to authenticate first!\r\n";
            send(client_fd, auth_response.c_str(), auth_response.length(), 0);
            continue;
        }

        if (message.rfind("PASS", 0) == 0) {
            handlePassCommand(client_fd, message);
        } else if (message.rfind("NICK", 0) == 0) {
            handleNickCommand(client_fd, message);
        } else if (message.rfind("USER", 0) == 0) {
            handleUserCommand(client_fd, message);
        } else if (message.rfind("JOIN", 0) == 0) {
            handleJoinCommand(client_fd, message);
        } else if (message.rfind("PRIVMSG", 0) == 0) {
            handlePrivMsgCommand(client_fd, message);
        } else {
            std::string wrong_command = "ERROR :Unknown command\r\n";
            send(client_fd, wrong_command.c_str(), wrong_command.length(), 0);
        }
    }
}

std::vector<std::string> Server::receiveMessage(int client_fd) {
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0) {
        std::cerr << "Failed to receive message from client " << client_fd << std::endl;
        return std::vector<std::string>();;
    } else if (bytes_received == 0) {
        close(client_fd);
        clients.erase(client_fd);
        std::cout << "Client " << client_fd << " disconnected" << std::endl;
        return std::vector<std::string>();;
    }

    buffer[bytes_received] = '\0';
    std::string data(buffer);
    std::vector<std::string> messages;
    size_t pos = 0;

    while ((pos = data.find("\r\n")) != std::string::npos) {
        messages.push_back(data.substr(0, pos));
        data.erase(0, pos + 2);
    }
	if (!data.empty()) {
        messages.push_back(data);  // Treat remaining data as a complete message
        data.clear();              // Clear the leftover data
    }

    return messages;
}

void Server::handlePassCommand(int client_fd, const std::string& message) {
    std::string pass = message.substr(5);
    pass.erase(pass.find_last_not_of("\r\n") + 1);

    if (pass == this->password) {
        clients[client_fd].setPassword(pass);
        std::cout << "Client " << client_fd << " provided correct password." << std::endl;
        std::string corr_pass_response = "Password correct\r\n";
        send(client_fd, corr_pass_response.c_str(), corr_pass_response.length(), 0);
        clients[client_fd].authenticate();
        std::string auth_response = "You are successfully authenticated\r\n";
        send(client_fd, auth_response.c_str(), auth_response.length(), 0);
    } else {
        std::string incorr_pass_response = "ERROR :Invalid password\r\n";
        send(client_fd, incorr_pass_response.c_str(), incorr_pass_response.length(), 0);
    }
}

void Server::handleNickCommand(int client_fd, const std::string& message) {
    std::string nick = message.substr(5);
	std::string::size_type pos = nick.find_last_not_of("\r\n");
    if (pos != std::string::npos) {
        nick.erase(pos + 1);
    }
    if (!clients[client_fd].isValidNickname(nick)) {
        std::string response = "ERROR: Invalid nickname. Symbols: : $ # & . , * ! ? @ are not allowed\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        std::string new_nick = clients[client_fd].sanitizeNickname(nick);
        clients[client_fd].setNickname(new_nick);
        std::cout << "Client " << client_fd << " set nickname to: " << new_nick << std::endl;
        std::string nick_set = "Your nick is set to " + new_nick + "\r\n";
        send(client_fd, nick_set.c_str(), nick_set.length(), 0);
    }
    if (!clients[client_fd].isUniqueNickname(nick, clients)) {
        std::string response = "ERROR: This nickname already exists\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        std::string u_nick = clients[client_fd].getUniqueNickname(nick, clients);
        clients[client_fd].setNickname(u_nick);
        std::cout << "Client " << client_fd << " set nickname to: " << u_nick << std::endl;
        std::string nick_set = "Your nick is set to " + u_nick + "\r\n";
        send(client_fd, nick_set.c_str(), nick_set.length(), 0);
    }
    else {
		clients[client_fd].authenticate();
        clients[client_fd].setNickname(nick);
        std::cout << "Client " << client_fd << " set nickname to: " << nick << std::endl;
        std::string nick_set = "Your nick is set to " + nick + "\r\n";
        send(client_fd, nick_set.c_str(), nick_set.length(), 0);
    }
}

void Server::handleUserCommand(int client_fd, const std::string& message) {
    size_t pos = message.find(' ', 5);
    if (pos != std::string::npos) {
        std::string username = message.substr(5, pos - 5);
        clients[client_fd].setUsername(username);
        clients[client_fd].authenticate();
        std::cout << "Client " << client_fd << " set username to: " << username << std::endl;
        std::string usrnm_set = "Your username is set to " + username + "\r\n";
        send(client_fd, usrnm_set.c_str(), usrnm_set.length(), 0);

    } else {
        std::string response = "USAGE: USER <username> <whatever> <servername> :<realname>\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
    }
}

void Server::handleJoinCommand(int client_fd, const std::string& message) {
    size_t pos = message.find(' ');
    if (pos != std::string::npos) {
        std::string channel = message.substr(pos + 1);
        if (!channel.empty() && channel[0] == '#') {
            channels[channel].push_back(client_fd);
            std::cout << "Client " << client_fd << " joined channel: " << channel << std::endl;
            
            std::string response = ":" + clients[client_fd].getNickname() + " JOIN " + channel + "\r\n";
            send(client_fd, response.c_str(), response.length(), 0);
        } else {
            std::string errorMsg = "ERROR :Invalid channel name\r\n";
            send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
        }
    } else {
        std::string errorMsg = "ERROR :JOIN command missing channel\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
    }
}

void Server::handlePrivMsgCommand(int client_fd, const std::string& message) {
    size_t pos = message.find(' ');
    size_t colon_pos = message.find(" :");

    if (pos != std::string::npos && colon_pos != std::string::npos) {
        std::string target = message.substr(pos + 1, colon_pos - pos - 1);
        std::string msgContent = message.substr(colon_pos + 2);

        if (target[0] == '#') {
            if (channels.find(target) != channels.end()) {
                for (std::vector<int>::iterator it = channels[target].begin(); it != channels[target].end(); ++it) {
                    if (*it != client_fd) {
                        std::string response = ":" + clients[client_fd].getNickname() + " PRIVMSG " + target + " :" + msgContent + "\r\n";
                        send(*it, response.c_str(), response.length(), 0);
                    }
                }
            } else {
                std::string errorMsg = "ERROR :No such channel\r\n";
                send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
            }
        } else {
                for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
                    if (it->second.getNickname() == target) {
                        std::string response = ":" + clients[client_fd].getNickname() + " PRIVMSG " + target + " :" + msgContent + "\r\n";
                        send(it->first, response.c_str(), response.length(), 0);
                        return;
                }
            }
            std::string errorMsg = "ERROR :No such nick/channel\r\n";
            send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
        }
    } else {
        std::string errorMsg = "ERROR :Invalid PRIVMSG command\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
    }
}


