#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

void Server::handleKickCommand(int client_fd, const std::string& message) {
    std::string error_msg;

    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::vector<std::string> params = extractParams(message);
    std::string channel_name = trim(params[0]);

    if (params.size() < 2) {
        // sendError(client_fd, ERR_NEEDMOREPARAMS, "KICK", "Not enough parameters.");
        sendInfoMessage(client_fd, ERR_NEEDMOREPARAMS, channel_name, "Not enough parameters.");
        return;
    }

    std::string target_nickname = trim(params[1]);
	std::string reason;
	if (params.size() > 2) {
    	for (size_t i = 1; i < params.size(); ++i) {
        	if (i > 1) {
            	reason += " ";
        	}
        	reason += params[i];
    	}
    	reason = trim(reason);
	} else {
		reason = "No reason provided";
	}

    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_name);
    if (channel_it == channels.end()) {
        sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; // Get the channel object

	if (!channel.isOperator(client_fd)) {
        // sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to kick.");
        sendInfoMessage(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to kick.");
        return;
    }

    int target_fd = -1;
    for (std::vector<int>::const_iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        int member_fd = *it;
        if (clients[member_fd].getNickname() == target_nickname) {
            target_fd = member_fd;
            break;
        }
    }

    if (target_fd == -1) {
        // sendError(client_fd, ERR_NOSUCHNICK, target_nickname, "No such nickname.");
        sendInfoMessage(client_fd, ERR_NOSUCHNICK, channel_name, "No such nickname.");
        return;
    }

    // // Perform the kick
	Client& kickerClient = clients[client_fd];    // The client who initiated the kick
    Client& targetClient = clients[target_fd];    // The client being kicked
    channel.kick(kickerClient, targetClient, reason);

    // Notify the target about the kick
    std::string kick_message = channel_name + ": " + clients[client_fd].getNickname() + " kicked you out of " + channel_name + "\r\n";
    send(target_fd, kick_message.c_str(), kick_message.length(), 0);
}
