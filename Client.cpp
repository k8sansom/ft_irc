#include "Client.hpp"

Client::Client() : authenticated(false) {}
Client::Client(int fd, std::string nickname) : fd(fd), nickname(nickname), authenticated(false) {}
Client::~Client() {}

std::string Client::getNickname() const {
    return nickname;
}

void Client::setNickname(const std::string& nickname) {
    if (isValidNickname(nickname)) {
        this->nickname = nickname;
    }
}

void Client::setPassword(const std::string& password) {
    this->password = password;
}

bool Client::checkPassword(const std::string& inputPassword) const {
    return inputPassword == this->password;
}

void Client::setUsername(const std::string& username) {
    this->username = username;
}

std::string Client::getUsername() const {
    return username;
}

int Client::getFd() const {
    return fd;
}

void Client::authenticate() {
    authenticated = true;
}

bool Client::isAuthenticated() const {
    return authenticated;
}

bool Client::isValidNickname(const std::string& nickname) {
    if (nickname.empty()) return false;
    if (nickname[0] == ':' || nickname[0] == '$' || nickname[0] == '#' || nickname[0] == '&') return false;
    for (size_t i = 0; i < nickname.length(); ++i) {
        if (nickname[i] == '.' || nickname[i] == ' ' || nickname[i] == ',' || nickname[i] == '*' ||
            nickname[i] == '!' || nickname[i] == '?' || nickname[i] == '@') {
            std::cout << "Your nickname contains a prohibited symbol: " << nickname[i] << std::endl;
            std::cout << "Please enter a valid nickname." << std::endl;
            return false;
        }
    }
    return true;
}
