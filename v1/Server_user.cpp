#include "Server.hpp"

void handleUserCommand(int client_fd, const std::string& message);
std::vector<std::string> extractUserParams(const std::string& message);
bool validateUserCommand(int client_fd, const std::vector<std::string>& params);
std::string sanitizeUsername(const std::string& username);
void sendUsernameConfirmation(int client_fd, const std::string& username, const std::string& realname);

void Server::handleUserCommand(int client_fd, const std::string& message) {
    if (!isCommandFormatValid(message, "USER")) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "*", "Not enough parameters");
        return;
    }

    if (!clients[client_fd].getUsername().empty()) {
        sendError(client_fd, ERR_ALREADYREGISTERED, "*", "You may not reregister");
        return;
    }

    std::vector<std::string> params = extractUserParams(message);
    if (params.size() < 4 || !validateUserCommand(client_fd, params)) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "*", "USAGE: USER <username> <whatever> <servername> :<realname>");
        return;
    }

    std::string username = sanitizeUsername(params[0]);
    std::string realname = params[3];

    clients[client_fd].setUsername(username);
    clients[client_fd].setRealname(realname);
    clients[client_fd].authenticate();

    sendUsernameConfirmation(client_fd, username, realname);
}

std::vector<std::string> Server::extractUserParams(const std::string& message) {
    std::vector<std::string> params;
    size_t first_space = message.find(' ');
    size_t second_space = message.find(' ', first_space + 1);
    size_t third_space = message.find(' ', second_space + 1);
    size_t colon_pos = message.find(" :", third_space + 1);

    if (first_space != std::string::npos && second_space != std::string::npos &&
        third_space != std::string::npos && colon_pos != std::string::npos) {
        params.push_back(message.substr(first_space + 1, second_space - first_space - 1)); // username
        params.push_back(message.substr(second_space + 1, third_space - second_space - 1)); // 0
        params.push_back(message.substr(third_space + 1, colon_pos - third_space - 1)); // *
        params.push_back(message.substr(colon_pos + 2)); // realname
    }

    return params;
}

bool Server::validateUserCommand(int client_fd, const std::vector<std::string>& params) {
    if (params.size() < 4) {
        return false;
    }

    std::string whatever_param = params[1];
    std::string servername_param = params[2];

    if (whatever_param.empty() || servername_param.empty()) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "*", "USAGE: USER <username> <whatever> <servername> :<realname>");
        return false;
    }


    return true;
}

std::string Server::sanitizeUsername(const std::string& username) {
    std::string sanitized_username = username;

    size_t max_userlen = 9;
    if (sanitized_username.length() > max_userlen) {
        sanitized_username = sanitized_username.substr(0, max_userlen);
    }
    return sanitized_username;
}

void Server::sendUsernameConfirmation(int client_fd, const std::string& username, const std::string& realname) {
    std::cout << "Client " << client_fd << " set username to: " << username << " and realname to: " << realname << std::endl;
    std::string confirmation = "Your username is set to " + username + " and your realname is " + realname + "\r\n";
    send(client_fd, confirmation.c_str(), confirmation.length(), 0);
}
