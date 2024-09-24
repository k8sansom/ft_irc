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

std::map<std::string, Channel>& Server::getChannels() {
    return channels;
}

std::string Server::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, last - first + 1);
}

void Server::handleClientMessage(int client_fd) {
    if (clients.find(client_fd) == clients.end()) {
        std::cerr << "ERROR: Client not found in the list" << std::endl;
        return;
    }

    std::vector<std::string> messages = receiveMessage(client_fd);
    if (messages.empty()) 
        return ;

    for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); ++it) {
        std::string message = *it;
        if (message.empty()) 
            continue;
        if (message.rfind("CAP", 0) == 0) {
            continue ;
        }
        std::cout << "Received from client " << client_fd << ": " << message << std::endl;

        if (!clients[client_fd].isAuthenticated() && message.rfind("PASS", 0) != 0) {
            std::string auth_response = "ERROR: You need to authenticate first!\r\n";
            send(client_fd, auth_response.c_str(), auth_response.length(), 0);
            close(client_fd);
            clients.erase(client_fd);
            std::cout << "Client " << client_fd << " disconnected" << std::endl;
            break ;
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
        }  else if (message.rfind("KICK", 0) == 0) {
            handleKickCommand(client_fd, message);
        } else if (message.rfind("INVITE", 0) == 0) {
            handleInviteCommand(client_fd, message);
        } /*else if (message.rfind("MODE", 0) == 0) {
            handleModeCommand(client_fd, message);
        } */else if (message.rfind("TOPIC", 0) == 0) {
            handleTopicCommand(client_fd, message);
        } else {
            std::string wrong_command = "ERROR: Unknown command\r\n";
            send(client_fd, wrong_command.c_str(), wrong_command.length(), 0);
        }
    }
}

std::vector<std::string> Server::receiveMessage(int client_fd) {
    char buffer[1024];
    int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0) {
        std::cerr << "ERROR: Failed to receive message from client " << client_fd << std::endl;
        return std::vector<std::string>();
    } else if (bytes_received == 0) {
        close(client_fd);
        clients.erase(client_fd);
        std::cout << "Client " << client_fd << " disconnected" << std::endl;
        return std::vector<std::string>();
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
        messages.push_back(data);
        data.clear();
    }

    return messages;
}

void Server::handlePassCommand(int client_fd, const std::string& message) {
    if (!isCommandFormatValid(message, "PASS")) {
        std::string err_msg = "USAGE: PASS <password>\r\n";
        send(client_fd, err_msg.c_str(), err_msg.length(), 0);
        return;
    }
    std::string pass = message.substr(5);
    pass.erase(pass.find_last_not_of("\r\n") + 1);

    if (pass == this->password) {
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

bool Server::isCommandFormatValid(const std::string& message, const std::string& command) {
    if (message.size() <= command.size() || message.substr(0, command.size()) != command || message[command.size()] != ' ') {
        return false;
    }
    return true;
}

void Server::sendError(int client_fd, int error_code, const std::string& target, const std::string& message) {
	std::ostringstream oss;
	oss << error_code;
    std::string error_msg = oss.str() + " " + target + ": " + message + "\r\n";
    send(client_fd, error_msg.c_str(), error_msg.length(), 0);
}

std::vector<std::string> Server::split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}