#include "Client.hpp"

Client::Client() : authenticated(false) {}
Client::Client(int fd, std::string nickname) : fd(fd), nickname(nickname), authenticated(false) {}
Client::~Client() {}

std::string Client::getNickname() const {
    return nickname;
}

void Client::setNickname(const std::string& nickname) {
        this->nickname = nickname;
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

bool Client::isValidNickname(const std::string& nickname) const {
    if (nickname.empty()) return false;
    if (nickname[0] == ':' || nickname[0] == '$' || nickname[0] == '#' || nickname[0] == '&') return false;
    for (size_t i = 0; i < nickname.length(); ++i) {
        if (nickname[i] == '.' || nickname[i] == ' ' || nickname[i] == ',' || nickname[i] == '*' ||
            nickname[i] == '!' || nickname[i] == '?' || nickname[i] == '@') {
            std::cout << "Your nickname contains a prohibited symbol: " << nickname[i] << std::endl;
            return false;
        }
    }
    return true;
}

std::string Client::sanitizeNickname(const std::string& nickname) const {
    if (nickname.empty()) return "";

    std::string sanitized_nickname;

    if (nickname[0] != ':' && nickname[0] != '$' && nickname[0] != '#' && nickname[0] != '&') {
        sanitized_nickname += nickname[0];
    } else {
        std::cout << "Your nickname starts with a prohibited character, removing it." << std::endl;
    }

    for (size_t i = 1; i < nickname.length(); ++i) {
        if (nickname[i] != '.' && nickname[i] != ' ' && nickname[i] != ',' && nickname[i] != '*' &&
            nickname[i] != '!' && nickname[i] != '?' && nickname[i] != '@') {
            sanitized_nickname += nickname[i];
        } else {
            std::cout << "Your nickname contains a prohibited symbol: " << nickname[i] << ", removing it." << std::endl;
        }
    }

    return sanitized_nickname;
}

bool Client::isUniqueNickname(const std::string& nickname, const std::map<int, Client>& clients) const {
    std::map<int, Client>::const_iterator it;
	for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNickname() == nickname) {
            std::cout << "Nickname '" << nickname << "' is already in use." << std::endl;
            return false;
        }
    }
    return true;
}

std::string Client::getUniqueNickname(const std::string& nickname, const std::map<int, Client>& clients) const {
    std::srand(std::time(0));

    std::string new_nickname = nickname;

    while (true) {
        bool is_unique = true;

        for (std::map<int, Client>::const_iterator it = clients.begin(); it != clients.end(); ++it) {
            if (it->second.getNickname() == new_nickname) {
                is_unique = false;
                break;
            }
        }

        if (is_unique) {
            return new_nickname;
        } else {
            int random_number = std::rand() % 1000;
            new_nickname = "user" + std::to_string(random_number) + "_" + nickname;
        }
    }
}
