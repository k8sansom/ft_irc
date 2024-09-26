#include "Channel.hpp"

Channel::Channel() : _name(""), _topic(""), _key("") {
	_inviteOnly = false;
	_topicRestricted = false;
	_keyReq = false;
	_userLimit = 0;
}

Channel::Channel(const std::string& channelName, int operator_fd) 
    : _name(channelName), _topic("") {
    _members.push_back(operator_fd);
	_operators.insert(operator_fd);
}
	
Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

const std::vector<int>& Channel::getMembers() const {
    return _members;
}

bool Channel::getMode(const std::string mode) const {
	if (mode == "topicRestricted") {
		return _topicRestricted;
	} else if (mode == "inviteOnly") {
		return _inviteOnly;
	} else if (mode == "keyReq") {
		return _keyReq;
	}  else {
		return false;
	}
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
    if (_operators.find(client_fd) != _operators.end()) {
        _operators.insert(_members.front());  // Transfer operator rights to the first member
    }
}

bool Channel::isEmpty() const {
    return _members.empty();
}

bool Channel::isOperator(int client_fd) const {
    // Use std::set::find to check if the client_fd exists in the _operators set
    return _operators.find(client_fd) != _operators.end();
}

void Channel::broadcastMessage(const std::string& message, int sender_fd) {
    std::cout << "Broadcasting message: " << message << " from sender: " << sender_fd << std::endl;
    
    // Iterate over all members in the channel
    for (std::vector<int>::iterator it = _members.begin(); it != _members.end(); ++it) {
        int member_fd = *it; 
        // Skip sending the message to the sender
        if (member_fd == sender_fd) {
            continue;
        }

        // Attempt to send the message
        ssize_t bytes_sent = send(member_fd, message.c_str(), message.length(), 0);

        // Check if the send was successful
        if (bytes_sent < 0) {
            std::cerr << "Error: Failed to send message to client " << member_fd << std::endl;

            // Optional: You may want to remove the member from the channel if the send fails consistently
        } else {
            std::cout << "Message sent to client " << member_fd << ": " << bytes_sent << " bytes" << std::endl;
        }
    }
}

bool Channel::checkChannelKey(const std::string key) const {
    // Check if the channel requires a key
    if (_keyReq) {
		if (key != _key) {
        	std::cout << "Channel is password protected." << std::endl;
        	return false; // Channel is password protected, no key provided
		}
    }
	return true;
}
bool Channel::checkInvite(int client_fd) const {
    // Check if invite-only mode is active
    if (_inviteOnly) {
        // Check if the client_fd is on the invite list
        if (_invitedClients.find(client_fd) == _invitedClients.end()) {
            std::cout << "You are not invited to this channel." << std::endl;
            return false; // Client is not invited
        }
    }
	return true;
}

bool Channel::checkUserLimit(void) const {
	if (_userLimit > 0) {
        // Get the current number of members in the channel
        if (_members.size() >= _userLimit) {
            std::cout << "User limit reached. Cannot join." << std::endl;
            return false; // User limit has been reached
        }
    }
    return true; // Client can join the channel
}

void Channel::kick(Client& targetClient, const std::string& reason) {
    std::vector<int>::iterator it = std::find(_members.begin(), _members.end(), targetClient.getFd());
    if (it != _members.end()) {
        _members.erase(it); // Remove from channel members
        std::cout << "KICK: " << targetClient.getNickname() << " has been kicked from the channel: " << reason << std::endl;
    } else {
        std::cout << "KICK: No such client found in the channel." << std::endl;
    }
}

void Channel::invite(Client& targetClient) {
    if (_invitedClients.find(targetClient.getFd()) == _invitedClients.end()) {
        _invitedClients.insert(targetClient.getFd()); // Mark as invited
        std::cout << "INVITE: " << targetClient.getNickname() << " has been invited to the channel." << std::endl;
    } else {
        std::cout << "INVITE: " << targetClient.getNickname() << " is already invited." << std::endl;
    }
}

void Channel::mode(const char flag, const std::string& param) {
    switch (flag) {
        case 'i':  // Invite-only mode
            _inviteOnly = !_inviteOnly;  // Toggle invite-only mode
            std::cout << "MODE: Invite-only mode " << (_inviteOnly ? "enabled" : "disabled") << std::endl;
            break;

        case 't':  // Topic restriction mode
            _topicRestricted = !_topicRestricted;  // Toggle topic restriction mode
            std::cout << "MODE: Topic-restricted mode " << (_topicRestricted ? "enabled" : "disabled") << std::endl;
            break;

        case 'k':  // Set channel key
            _key = param;  // Set the key directly
			_keyReq = true;
            std::cout << "MODE: Channel key set to: " << _key << std::endl;
            break;

        case 'l':  // Set user limit
            {
                std::stringstream ss1(param);  // Initialize within the scope
                ss1 >> _userLimit;  // Set the user limit
                std::cout << "MODE: User limit set to: " << _userLimit << std::endl;
            }
            break;

        case 'o': {  // Grant operator privileges
            int clientFd;
            std::stringstream ss2(param);
            if (ss2 >> clientFd) {
				if (!isOperator(clientFd)) {
						_operators.insert(clientFd);  // Grant operator privilege
						std::cout << "MODE: Client with fd " << clientFd << " is now an operator." << std::endl;
                } else {
                    _operators.erase(clientFd);
					std::cout << "MODE: Client with fd " << clientFd << " is no longer an operator." << std::endl;
                }
            } else {
                std::cout << "MODE: Invalid client fd parameter." << std::endl;
            }
            break;
        }

        default:
            std::cout << "MODE: Unknown mode flag." << std::endl;
            break;
    }
}

void Channel::topic(const std::string& newTopic) {
    _topic = newTopic;
    std::cout << "TOPIC: The channel topic has been changed to: " << newTopic << "\r\n";
}

std::string Channel::getTopic() const {
    return _topic.empty() ? "No topic set" : "Current topic: " + _topic;
}


