#include "Client.hpp"

Client::Client() {}
Client::Client(int fd, std::string nickname) : fd(fd), nickname(nickname), authentificated(false) {}
Client::~Client() {}

std::string Client::getNickname() const {
    return nickname;
}

void Client::setNickname(const std::string& nickname) {
    if (isValidNickname(nickname)) {
        this->nickname = nickname;
    }
}

int Client::getFd() const {
    return fd;
}

void Client::authentificate() {
    authentificated = true;
}

bool Client::isAuthenticated() const {
    return authentificated;
}

bool Client::isValidNickname(const std::string& nickname) {
    if (nickname.empty())
        return false;
    if (nickname[0] == ':' || nickname[0] == '$' || nickname[0] == '#' || nickname[0] == '&')
        return false;
    for (int i = 0; nickname[i]; ++i) {
        if (nickname[i] == '.' || nickname[i] == ' ' || nickname[i] == ',' || nickname[i] == '*'
            || nickname[i] == '!' || nickname[i] == '?' || nickname[i] == '@') {
                std::cout << "Your nickname contains prohibited symbol: " << nickname[i] << std::endl;
                std::cout << "Please enter a valid nickname" << std::endl;
                return false;
            }          
    }
    return true;
}
