#include "Channel.hpp"

Channel::Channel() : _name(""), _operator_fd(-1), _topic(""), _key("") {
    _mode.inviteOnly = false;
    _mode.topicRestricted = false;
    _mode.userLimit = 0;
}

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

bool Channel::canClientJoin(const std::string& key) const {
    return (_key.empty() || _key == key);
}

void Channel::kick(Client& operatorClient, Client& targetClient, const std::string& reason) {
    if (!isOperator(operatorClient.getFd())) {
        std::cout << "KICK: " << operatorClient.getNickname() << " does not have permission to KICK" << std::endl;
        return;
    }

    std::vector<int>::iterator it = std::find(_members.begin(), _members.end(), targetClient.getFd());
    if (it != _members.end()) {
        _members.erase(it); // Remove from channel members
        std::cout << "KICK: " << targetClient.getNickname() << " has been kicked from the channel: " << reason << std::endl;
    } else {
        std::cout << "KICK: No such client found in the channel." << std::endl;
    }
}

void Channel::invite(Client& operatorClient, Client& targetClient) {
    if (!isOperator(operatorClient.getFd()) && _mode.inviteOnly) {
        std::cout << "INVITE: " << operatorClient.getNickname() << " does not have permission to invite users." << std::endl;
        return;
    }

    if (_invitedClients.find(targetClient.getFd()) == _invitedClients.end()) {
        _invitedClients.insert(targetClient.getFd()); // Mark as invited
        std::cout << "INVITE: " << targetClient.getNickname() << " has been invited to the channel." << std::endl;
    } else {
        std::cout << "INVITE: " << targetClient.getNickname() << " is already invited." << std::endl;
    }
}

// void Channel::setMode(Client& operatorClient, const std::string& modeFlags, const std::string& param) {
//     if (!isOperator(operatorClient.getFd())) {
//         std::cout << "MODE: You don't have permission to set the mode." << std::endl;
//         return;
//     }

//     for (size_t i = 0; i < modeFlags.length(); ++i) {
//     	char flag = modeFlags[i];
//         switch (flag) {
//             case 'i':
//                 _mode.inviteOnly = !_mode.inviteOnly;
//                 std::cout << "MODE: Invite-only mode " << (_mode.inviteOnly ? "enabled" : "disabled") << std::endl;
//                 break;
//             case 't':
//                 _mode.topicRestricted = !_mode.topicRestricted;
//                 std::cout << "MODE: Topic-restriction " << (_mode.topicRestricted ? "enabled" : "disabled") << std::endl;
//                 break;
//             case 'k':
//                 _mode.channelKey = param;
//                 std::cout << "MODE: Channel key set to: " << param << std::endl;
//                 break;
//             case 'l':
//                 std::stringstream ss1(param);
// 				ss1 >> _mode.userLimit;
//                 std::cout << "MODE: User limit set to: " << _mode.userLimit << std::endl;
//                 break;
//             case 'o':
//                 std::stringstream ss2(param);
// 				ss1 >> clientFd;
//                 if (std::find(_members.begin(), _members.end(), clientFd) != _members.end()) {
//                     _operators.insert(clientFd);  // Grant operator privilege
//                     std::cout << "MODE: Client with fd " << clientFd << " is now an operator." << std::endl;
//                 } else {
//                     std::cout << "MODE: No such client found." << std::endl;
//                 }
//                 break;
//             default:
//                 std::cout << "MODE: Unknown mode flag." << std::endl;
//                 break;
//         }
//     }
// }

void Channel::setTopic(Client& operatorClient, const std::string& newTopic) {
    if (_mode.topicRestricted && !isOperator(operatorClient.getFd())) {
        std::cout << "TOPIC: " << operatorClient.getNickname() << " does not have permission to change the topic.\r\n";
        return;
    }
    _topic = newTopic;
    std::cout << "TOPIC: The channel topic has been changed to: " << newTopic << "\r\n";
}

std::string Channel::getTopic() const {
    return _topic.empty() ? "No topic set" : "Current topic: " + _topic;
}


