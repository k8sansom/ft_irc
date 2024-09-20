#include "Client.hpp"

Client::Client() : _authenticated(false) {}
Client::Client(int fd, std::string nickname) : _fd(fd), _nickname(nickname), _authenticated(false) {}
Client::~Client() {}

std::string Client::getNickname() const {
    return _nickname;
}

void Client::setNickname(const std::string& nickname) {
        this->_nickname = nickname;
}

void Client::setUsername(const std::string& username) {
    this->_username = username;
}

std::string Client::getUsername() const {
    return _username;
}

int Client::getFd() const {
    return _fd;
}

std::string Client::getRealName() const {
    return _realname;
}

void Client::setRealname(const std::string& realname) {
    this->_realname = realname;
}

void Client::authenticate() {
    _authenticated = true;
}

bool Client::isAuthenticated() const {
    return _authenticated;
}

bool Client::isValidNickname(const std::string& nickname) const {
    if (nickname.empty()) return false;
    if (nickname[0] == ':' || nickname[0] == '$' || nickname[0] == '#' || nickname[0] == '&') return false;
    for (size_t i = 0; i < nickname.length(); ++i) {
        if (nickname[i] == '.' || nickname[i] == ' ' || nickname[i] == ',' || nickname[i] == '*' ||
            nickname[i] == '!' || nickname[i] == '?' || nickname[i] == '@') {
            return false;
        }
    }
    return true;
}

std::string Client::sanitizeNickname(const std::string& nickname, std::string &err_msg) const {
    if (nickname.empty()) 
        return "";

    std::string sanitized_nickname;

    if (nickname[0] != ':' && nickname[0] != '$' && nickname[0] != '#' && nickname[0] != '&') {
        sanitized_nickname += nickname[0];
    } else {
        err_msg += "Your nickname starts with a prohibited character: " + std::string(1, nickname[0]) + ", removing it.\r\n";
    }

    for (size_t i = 1; i < nickname.length(); ++i) {
        if (nickname[i] != '.' && nickname[i] != ' ' && nickname[i] != ',' && nickname[i] != '*' &&
            nickname[i] != '!' && nickname[i] != '?' && nickname[i] != '@') {
            sanitized_nickname += nickname[i];
        } else {
            err_msg += "Your nickname contains a prohibited symbol: " + std::string(1, nickname[i]) + ", removing it.\r\n";
        }
    }

    if (sanitized_nickname.empty()) {
		std::ostringstream oss;
		oss << (std::rand() % 1000);
    	std::string sanitized_nickname = "user" + oss.str();  // Concatenate with "user"
        // sanitized_nickname = "user" + std::to_string(std::rand() % 1000);
        err_msg += "Your nickname was invalid. Setting it to a default: " + sanitized_nickname + "\r\n";
    }

    return sanitized_nickname;
}



bool Client::isUniqueNickname(const std::string& nickname, const std::map<int, Client>& clients, std::string &err_msg) const {
    std::map<int, Client>::const_iterator it;
	for (it = clients.begin(); it != clients.end(); ++it) {
        if (it->second.getNickname() == nickname) {
            err_msg = "Nickname '" + nickname + "' is already in use.\r\n";
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
			std::stringstream ss;
			ss << random_number;
			new_nickname = nickname + "_" + ss.str();
        }
    }
}

bool Client::checkAttributes(std::string& error_msg) const {

    if (!this->_authenticated) {
        error_msg = "ERROR :You need to authenticate first!\r\n";
        return false;
    }

    if (this->_nickname.empty()) {
        error_msg = "ERROR :You must set a nickname before proceeding\r\n";
        return false;
    }

    if (this->_username.empty()) {
        error_msg = "ERROR :You must set a username before proceeding\r\n";
        return false;
    }

    return true;
}

