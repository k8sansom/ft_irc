#include "Channels.hpp"

Channel::Channel(const std::string& channelName) : _name(channelName) {}

Channel::~Channel(){}

const std::string& Channel::getName() const {
        return _name;
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
}

bool Channel::isEmpty() const {
        return _members.empty();
}

void Channel::broadcastMessage(const std::string& message, int sender_fd) {
	for (int member_fd : _members) {
		if (member_fd != sender_fd) {
			send(member_fd, message.c_str(), message.length(), 0);
        }
    }
}