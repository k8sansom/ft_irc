#include "Server.hpp"

bool validateFormat(int client_fd, const std::string& message);
bool checkAlreadySet(int client_fd, const std::string& nick);
bool checkUnique(int client_fd, std::string& nick, std::string& err_msg);
void setNickname(int client_fd, const std::string& nick);
void handleNickCommand(int client_fd, const std::string& message);
bool sanitizeNickname(int client_fd, std::string& nick, std::string& err_msg);


bool Server::validateFormat(int client_fd, const std::string& message) {
    if (!isCommandFormatValid(message, "NICK")) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "NICK", "Usage: NICK <nickname>");
        return false;
    }
    return true;
}

bool Server::checkAlreadySet(int client_fd, const std::string& nick) {
    if (clients[client_fd].getNickname() == nick) {
        sendError(client_fd, ERR_NICKNAMEINUSE, nick, "This nickname is already set.");
        return true;
    }
    return false;
}

bool Server::sanitizeNickname(int client_fd, std::string& nick, std::string& err_msg) {
    if (!clients[client_fd].isValidNickname(nick)) {
        std::string new_nick = clients[client_fd].sanitizeNickname(nick, err_msg);
        if (!err_msg.empty()) {
            send(client_fd, err_msg.c_str(), err_msg.length(), 0);
        }
        clients[client_fd].setNickname(new_nick);
        std::cout << "Client " << client_fd << " set nickname to: " << new_nick << std::endl;
        std::string nick_set = "Your nick is set to " + new_nick + "\r\n";
        send(client_fd, nick_set.c_str(), nick_set.length(), 0);
        return true;
    }
    return false;
}

bool Server::checkUnique(int client_fd, std::string& nick, std::string& err_msg) {
    if (!clients[client_fd].isUniqueNickname(nick, clients, err_msg)) {
        sendError(client_fd, ERR_NICKNAMEINUSE, nick, err_msg);
        std::string u_nick = clients[client_fd].getUniqueNickname(nick, clients);
        clients[client_fd].setNickname(u_nick);
        std::cout << "Client " << client_fd << " set nickname to: " << u_nick << std::endl;
        std::string nick_set = "Your nick is set to " + u_nick + "\r\n";
        send(client_fd, nick_set.c_str(), nick_set.length(), 0);
        return true;
    }
    return false;
}

void Server::setNickname(int client_fd, const std::string& nick) {
    clients[client_fd].authenticate();
	std::string final_nick = trim(nick);
    clients[client_fd].setNickname(final_nick);
    std::cout << "Client " << client_fd << " set nickname to: " << final_nick << std::endl;
    std::string nick_set = "Your nick is set to " + final_nick + "\r\n";
    send(client_fd, nick_set.c_str(), nick_set.length(), 0);
}

void Server::handleNickCommand(int client_fd, const std::string& message) {
    if (!validateFormat(client_fd, message)) 
        return;

    std::string nick = message.substr(5);
    std::string::size_type pos = nick.find_last_not_of("\r\n");
    if (pos != std::string::npos) {
        nick.erase(pos + 1);
    }

    std::string err_msg;

    if (checkAlreadySet(client_fd, nick))
        return;
    if (sanitizeNickname(client_fd, nick, err_msg))
        return;
    if (checkUnique(client_fd, nick, err_msg))
        return;
    setNickname(client_fd, nick);
}

