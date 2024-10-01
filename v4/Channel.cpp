#include "Channel.hpp"

Channel::Channel() 
    : _name(""), 
      _topic(""), 
      _key(""), 
      _inviteOnly(false), 
      _topicRestricted(false), 
      _keyReq(false), 
      _userLimit(0)
{}

Channel::Channel(const std::string& channelName, int operator_fd) 
    : _name(channelName),
      _topic(""), 
      _key(""), 
      _inviteOnly(false), 
      _topicRestricted(false), 
      _keyReq(false), 
      _userLimit(0) {
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

void Channel::broadcastMessage(const std::string& message, int exclude_fd) {
    std::cout << "Broadcasting message: " << message << std::endl;

    for (std::vector<int>::iterator it = _members.begin(); it != _members.end(); ++it) {
        int member_fd = *it;

        // Skip sending to the excluded client, unless exclude_fd is -1
        if (exclude_fd != -1 && member_fd == exclude_fd) {
            continue;
        }

        ssize_t bytes_sent = send(member_fd, message.c_str(), message.length(), 0);

        if (bytes_sent < 0) {
            std::cerr << "Error: Failed to send message to client " << member_fd << std::endl;
        } else {
            std::cout << "Message sent to client " << member_fd << ": " << bytes_sent << " bytes" << std::endl;
        }
    }
}


bool Channel::checkChannelKey(const std::string key) const {
    if (key != _key) {
        std::cout << "Channel password is incorrect." << std::endl;
        return false; // Channel is password protected, no key provided
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

void Channel::kick(Client& kickerClient, Client& targetClient, const std::string& reason) {
    std::vector<int>::iterator it = std::find(_members.begin(), _members.end(), targetClient.getFd());
    if (it != _members.end()) {
        std::string kick_message = ":" + kickerClient.getNickname() + "!" + targetClient.getUsername() + "@server KICK " + _name + " " + targetClient.getNickname() + " :" + reason + "\r\n";
        broadcastMessage(kick_message, -1);
  
        std::cout << "KICK: " << targetClient.getNickname() << " has been kicked from the channel: " << reason << std::endl;
    
        _members.erase(it);
    } else {
        std::string feedback_message = ":" + kickerClient.getNickname() + "!" + kickerClient.getUsername() + "@server NOTICE " + kickerClient.getNickname() + " :No such client found in channel " + _name + "\r\n";
        send(kickerClient.getFd(), feedback_message.c_str(), feedback_message.length(), 0);
        std::cout << "KICK: No such client found in the channel." << std::endl;
    }
}


void Channel::invite(Client& inviterClient, Client& targetClient) {
    if (_invitedClients.find(targetClient.getFd()) == _invitedClients.end()) {
        _invitedClients.insert(targetClient.getFd()); // Mark as invited

        std::string invite_message = ":" + inviterClient.getNickname() + " INVITE " + targetClient.getNickname() + " :" + _name + "\r\n";
        broadcastMessage(invite_message, -1);

        std::cout << "INVITE: " << targetClient.getNickname() << " has been invited by " << inviterClient.getNickname() << " to the channel." << std::endl;
    } else {
        std::string already_invited_msg = ":" + inviterClient.getNickname() + " NOTICE " + _name + " :" + targetClient.getNickname() + " has already been invited to the channel.\r\n";
        send(inviterClient.getFd(), already_invited_msg.c_str(), already_invited_msg.length(), 0);
        
        std::cout << "INVITE: " << targetClient.getNickname() << " is already invited." << std::endl;
    }
}


void Channel::mode(const char flag, const char sign, const std::string& param) {
    switch (flag) {
		case 'i':  // Invite-only mode
            if (sign == '+') {
				_inviteOnly = true;
				std::cout << "MODE: Invite-only mode enabled" << std::endl;
			} else {
				_inviteOnly = false;
				std::cout << "MODE: Invite-only mode disabled" << std::endl;
			}

    	case 't':  // Topic restriction mode
            if (sign == '+') {
				_topicRestricted = true;
				std::cout << "MODE: Topic restricted mode enabled" << std::endl;
			} else {
				_topicRestricted = false;
				std::cout << "MODE: Topic restricted mode disabled" << std::endl;
			}

        case 'k':  // Set channel key
			if (sign == '+') {
				_key = param;  // Set the key directly
				_keyReq = true;
				std::cout << "MODE: Channel key set to: " << _key << std::endl;
			} else {
				_key = "";
				_keyReq = false;
				std::cout << "MODE: Channel key requirement removed" << std::endl;
			}
        	break;

        case 'l':  // Set user limit
			if (sign == '+') {
				std::stringstream ss1(param);
				ss1 >> _userLimit;  // Set the user limit
				std::cout << "MODE: User limit set to: " << _userLimit << std::endl;
			} else {
				_userLimit = 0;
			}
			break;

        case 'o': {  // Grant operator privileges
            int clientFd;
			std::stringstream ss2(param);
            if (ss2 >> clientFd) {
				if (sign == '+') {
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

std::string Channel::getCurrentModeFlags() const {
    std::string modes;

    if (_inviteOnly) {
        modes += 'i';  // Invite-only
    }
    if (_topicRestricted) {
        modes += 't';  // Topic-restricted
    }
    if (_keyReq) {
        modes += 'k';  // Channel key required
    }
    if (_userLimit > 0) {
        modes += 'l';  // User limit is set
    }

    return modes.empty() ? "" : modes;
}

void Channel::topic(const std::string& newTopic) {
    _topic = newTopic;
    std::cout << "TOPIC: The channel topic has been changed to: " << newTopic << "\r\n";
}

std::string Channel::getTopic() const {
    return _topic.empty() ? "No topic set" : "Current topic: " + _topic;
}
