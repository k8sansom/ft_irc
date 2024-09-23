#include "Channel.hpp"

Channel::Channel() :  _name(""), _members(), _operator_fd(-1), _topic(""), _key("") {}

Channel::Channel(const std::string& channelName, int operator_fd) 
    : _name(channelName), _operator_fd(operator_fd), _topic("") {
    _members.push_back(operator_fd);
}

Channel::Channel(const std::string& channelName, int operator_fd, const std::string& key)
    : _name(channelName), _operator_fd(operator_fd), _key(key) {
    _members.push_back(operator_fd);
}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

const std::vector<int>& Channel::getMembers() const {
    return _members;
}

const std::string& Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

bool Channel::addClient(int client_fd) {
    if (std::find(_members.begin(), _members.end(), client_fd) == _members.end()) {
        _members.push_back(client_fd);
        return true;
    }
    return false;  // Client already in the channel
}

void Channel::removeClient(int client_fd) {
    _members.erase(std::remove(_members.begin(), _members.end(), client_fd), _members.end());
    if (client_fd == _operator_fd && !_members.empty()) {
        _operator_fd = _members.front();  // Transfer operator rights to the first member
    }
}

void Channel::setKey(const std::string& key) {
    this->_key = key;
}

bool Channel::isEmpty() const {
    return _members.empty();
}

bool Channel::isOperator(int client_fd) const {
    return _operator_fd == client_fd;
}

void Channel::broadcastMessage(const std::string& message, int sender_fd) {
    for (std::vector<int>::iterator it = _members.begin(); it != _members.end(); ++it) {
        if (*it != sender_fd) {
            if (send(*it, message.c_str(), message.length(), 0) < 0) {
                std::cerr << "Failed to send message to client " << *it << std::endl;
            }
        }
    }
}

bool Channel::canClientJoin(const std::string& key) const {
    return (_key.empty() || _key == key);
}
