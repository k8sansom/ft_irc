#include "Server.hpp"
#include "Channel.hpp"

void handleKickCommand(int client_fd, const std::string& message);
std::vector<std::string> extractParams(const std::string& message);
void leaveAllChannels(int client_fd);
bool isValidChannelName(const std::string& name);
void joinExistingChannel(int client_fd, const std::string& channel_name, const std::string& key);
void createAndJoinChannel(int client_fd, const std::string& channel_name, const std::string& key);
void sendJoinConfirmation(int client_fd, const std::string& channel_name);
void sendChannelInfo(int client_fd, const std::string& channel_name);


void Server::handleKickCommand(int client_fd, const std::string& message) {
    std::string error_msg;
    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Extract parameters from the message
    std::vector<std::string> params = extractParams(message);

    // Check for sufficient parameters
    if (params.size() < 2) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "KICK", "Not enough parameters.");
        return;
    }

    std::string channel_name = params[0];
    std::string target_nickname = params[1];
    std::string reason = (params.size() > 2) ? params[2] : "No reason specified";

    // Check if the channel exists
    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_name);
    if (channel_it == channels.end()) {
        sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; // Get the channel object

    // Check if the client is an operator in the channel
    if (!channel.isOperator(client_fd)) {
        sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to kick.");
        return;
    }

    // Find the target client by nickname
    int target_fd = -1;
    for (std::vector<int>::const_iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        int member_fd = *it;
        if (clients[member_fd].getNickname() == target_nickname) {
            target_fd = member_fd;
            break;
        }
    }

    if (target_fd == -1) {
        sendError(client_fd, ERR_NOSUCHNICK, target_nickname, "No such nick.");
        return;
    }

    // Perform the kick
    channel.kick(clients[client_fd], clients[target_fd], reason);

    // Notify the channel about the kick
    std::string kick_message = ":" + clients[client_fd].getNickname() + " KICK " + channel_name + " " + target_nickname + " :" + reason + "\r\n";
    channel.broadcastMessage(kick_message, client_fd);
}
