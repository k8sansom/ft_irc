#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

void Server::handleInviteCommand(int client_fd, const std::string& message) {
    std::string error_msg;
    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Extract parameters from the message
    std::vector<std::string> params = extractParams(message);

    // Check for sufficient parameters
    if (params.size() < 2) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "INVITE", "Not enough parameters.");
        sendInfoMessage(client_fd, ERR_NEEDMOREPARAMS, "INVITE", "Not enough parameters.");
        return;
    }

	std::string target_nickname = trim(params[0]);
	std::string channel_name = trim(params[1]);

    // Check if the channel exists
    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_name);
    if (channel_it == channels.end()) {
        sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; // Get the channel object

    // Check if the client has permission to invite (is an operator)
    if (!channel.isOperator(client_fd)) {
        // sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to invite.");
        sendInfoMessage(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to invite.");
        return;
    }

    // Find the target client
    int target_fd = -1;
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        if (clients[it->first].getNickname() == target_nickname) {
            target_fd = it->first;
            break;
        }
    }

    // If no client with the target nickname was found, send an error to the inviter
    if (target_fd == -1) {
        // sendError(client_fd, ERR_NOSUCHNICK, target_nickname, "No such nickname.");
        sendInfoMessage(client_fd, ERR_NOSUCHNICK, channel_name, "No such nickname.");
        return;
    }

    // Check if the target client is already in the channel
    const std::vector<int>& members = channel.getMembers();
    if (std::find(members.begin(), members.end(), target_fd) != members.end()) {
        std::string already_msg = ":" + clients[client_fd].getNickname() + " NOTICE " + channel_name + " :" + target_nickname + " is already in the channel.\r\n";
        send(client_fd, already_msg.c_str(), already_msg.length(), 0);
        return;
    }

    // Perform the invite
    channel.invite(clients[client_fd], clients[target_fd]);

    // Notify the target client about the invite
    std::string invite_message = ":" + clients[client_fd].getNickname() + " INVITE " + target_nickname + " :" + channel_name + "\r\n";
    send(target_fd, invite_message.c_str(), invite_message.length(), 0);

    // Notify the inviter about the successful invite
    std::string confirmation_message = "341 " + clients[client_fd].getNickname() + " " + target_nickname + " " + channel_name + "\r\n";
    send(client_fd, confirmation_message.c_str(), confirmation_message.length(), 0);
}
