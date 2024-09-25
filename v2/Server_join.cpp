#include "Server.hpp"
#include "Channel.hpp"

void handleJoinCommand(int client_fd, const std::string& message);
std::vector<std::string> extractParams(const std::string& message);
void leaveAllChannels(int client_fd);
bool isValidChannelName(const std::string& name);
void joinExistingChannel(int client_fd, const std::string& channel_name, const std::string& key);
void createAndJoinChannel(int client_fd, const std::string& channel_name, const std::string& key);
void sendJoinConfirmation(int client_fd, const std::string& channel_name);
void sendChannelInfo(int client_fd, const std::string& channel_name);


void Server::handleJoinCommand(int client_fd, const std::string& message) {
    std::string error_msg;
    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::vector<std::string> params = extractParams(message);
    if (params.empty() || params[0] == "0") {
        leaveAllChannels(client_fd);
        return;
    }

    if (params.size() < 1) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "JOIN", "Not enough parameters.");
        return;
    }

    // Use a different name for the list of channel names
    std::vector<std::string> requestedChannels = split(params[0], ',');
    std::vector<std::string> keys = params.size() > 1 ? split(params[1], ',') : std::vector<std::string>();

    for (size_t i = 0; i < requestedChannels.size(); ++i) {
        std::string channel_name = trim(requestedChannels[i]);
        std::string key = (i < keys.size()) ? keys[i] : "";

        if (!isValidChannelName(channel_name)) {
            sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "Invalid channel name.");
            continue;
        }
        // Check if the channel exists in the channels map
        std::map<std::string, Channel>::iterator it = channels.find(channel_name);
        if (it != channels.end()) {
            joinExistingChannel(client_fd, channel_name, key);
        } else {
            createAndJoinChannel(client_fd, channel_name, key);
        }
	}
}

std::vector<std::string> Server::extractParams(const std::string& message) {
    size_t pos = message.find(' ');
    if (pos == std::string::npos)
        return std::vector<std::string>();
    std::string params = message.substr(pos + 1);
    return split(params, ' ');
}

void Server::leaveAllChannels(int client_fd) {
    for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
        it->second.removeClient(client_fd);
        std::string response = ":" + clients[client_fd].getNickname() + " PART " + it->first + "\r\n";
        send(client_fd, response.c_str(), response.length(), 0);
        it->second.broadcastMessage(response, client_fd);
    }
}

bool Server::isValidChannelName(const std::string& name) {
    return !name.empty() && name[0] == '#';
}

void Server::joinExistingChannel(int client_fd, const std::string& channel_name, const std::string& key) {
    Channel& channel = channels[channel_name];

    if (!channel.checkChannelKey(key)) {
        sendError(client_fd, ERR_BADCHANNELKEY, channel_name, "Channel key is incorrect!");
        return;
    } else if (!channel.checkUserLimit()) {
		sendError(client_fd, ERR_BADCHANNELKEY, channel_name, "Channel user limit has been reached!");
        return;
	} else if (!channel.checkInvite(client_fd)) {
		sendError(client_fd, ERR_BADCHANNELKEY, channel_name, "Channel is invite only!");
        return;
	}

    if (channel.addClient(client_fd)) {
        sendJoinConfirmation(client_fd, channel_name);
        sendChannelInfo(client_fd, channel_name);
        channel.broadcastMessage(":" + clients[client_fd].getNickname() + " JOIN " + channel_name + "\r\n", client_fd);
    } else {
        std::string errorMsg = "ERROR: You are already in the channel\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
    }
}

void Server::createAndJoinChannel(int client_fd, const std::string& channel_name, const std::string& key) {
    channels.insert(std::make_pair(channel_name, Channel(channel_name, client_fd, key)));
    std::cout << "Channel created: " << channel_name << " with operator: " << client_fd << std::endl;

	std::cout << "Available channels after creation: ";
	for (std::map<std::string, Channel>::iterator it = channels.begin(); it != channels.end(); ++it) {
		std::cout << it->first << " ";
	}
	std::cout << std::endl;
	
    sendJoinConfirmation(client_fd, channel_name);
    sendChannelInfo(client_fd, channel_name);
}

void Server::sendJoinConfirmation(int client_fd, const std::string& channel_name) {
    std::string response = ":" + clients[client_fd].getNickname() + " JOIN " + channel_name + "\r\n";
    send(client_fd, response.c_str(), response.length(), 0);
}

void Server::sendChannelInfo(int client_fd, const std::string& channel_name) {
    Channel& channel = channels[channel_name];
    if (!channel.getTopic().empty()) {
        std::string topic = "332 " + channel_name + " :" + channel.getTopic() + "\r\n";
        send(client_fd, topic.c_str(), topic.length(), 0);
    }

    std::string user_list = "353 " + channel_name + " :";
    const std::vector<int>& members = channel.getMembers();
    for (size_t i = 0; i < members.size(); ++i) {
        user_list += clients[members[i]].getNickname() + " ";
    }
    user_list += "\r\n";
    send(client_fd, user_list.c_str(), user_list.length(), 0);

    std::string end_of_names = channel_name + ": " + clients[client_fd].getNickname() + " is now a channel member\r\n";
    send(client_fd, end_of_names.c_str(), end_of_names.length(), 0);
}
