#include "Server.hpp"

void handlePrivMsgCommand(int client_fd, const std::string& message);
std::pair<std::string, std::string> extractTargetAndMsg(const std::string& message, int client_fd);
void handleChannelMsg(int client_fd, const std::string& target, const std::string& msgContent);
void handleUserMsg(int client_fd, const std::string& target, const std::string& msgContent);


void Server::handlePrivMsgCommand(int client_fd, const std::string& message) {
    std::string error_msg;

    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::pair<std::string, std::string> targetAndMessage = extractTargetAndMsg(message, client_fd);
    if (targetAndMessage.first.empty() || targetAndMessage.second.empty()) {
        return ;
    }

    if (targetAndMessage.first[0] == '#') {
        handleChannelMsg(client_fd, targetAndMessage.first, targetAndMessage.second);
    } else {
        handleUserMsg(client_fd, targetAndMessage.first, targetAndMessage.second);
    }
}

std::pair<std::string, std::string> Server::extractTargetAndMsg(const std::string& message, int client_fd) {
    size_t pos = message.find(' ');
    size_t colon_pos = message.find(" :");

    if (pos == std::string::npos || colon_pos == std::string::npos || colon_pos <= pos + 1) {
        std::string errorMsg = "ERROR :Invalid PRIVMSG command\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
        return std::make_pair("", "");
    }

    std::string target = message.substr(pos + 1, colon_pos - pos - 1);
    std::string msgContent = message.substr(colon_pos + 2);

    if (msgContent.empty()) {
        std::string errorMsg = "ERROR :No text to send\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
        return std::make_pair("", "");
    }

    return std::make_pair(target, msgContent);
}


void Server::handleChannelMsg(int client_fd, const std::string& target, const std::string& msgContent) {
    if (channels.find(target) != channels.end()) {
        const std::vector<int>& members = channels[target].getMembers();
        if (std::find(members.begin(), members.end(), client_fd) == members.end()) {
            std::string errorMsg = "ERROR :You are not in channel " + target + "\r\n";
            send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
            return;
        }
        for (std::vector<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
            if (*it != client_fd) {
                std::string response = ":" + clients[client_fd].getNickname() + " PRIVMSG " + target + " :" + msgContent + "\r\n";
                send(*it, response.c_str(), response.length(), 0);
            }
        }
    } else {
        std::string errorMsg = "404 " + clients[client_fd].getNickname() + " " + target + " :Cannot send to channel\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
    }
}

void Server::handleUserMsg(int client_fd, const std::string& target, const std::string& msgContent) {
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNickname() == target) {
            std::string response = ":" + clients[client_fd].getNickname() + " PRIVMSG " + target + " :" + msgContent + "\r\n";
            send(it->first, response.c_str(), response.length(), 0);
            return;
        }
    }
    std::string errorMsg = "401 " + clients[client_fd].getNickname() + " " + target + " :No such nick/channel\r\n";
    send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
}
