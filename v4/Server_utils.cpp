#include "Server.hpp"

std::string Server::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, last - first + 1);
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
