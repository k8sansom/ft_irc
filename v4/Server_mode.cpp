#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

bool isNonNegativeInteger(const std::string& str) {
    if (str.empty()) return false; // Check for empty string
    for (size_t i = 0; i < str.length(); ++i) {
        if (!isdigit(str[i])) return false; // Check if each character is a digit
    }
    return true; // All characters are digits
}

void Server::handleModeCommand(int client_fd, const std::string& message) {
    std::string error_msg;
    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Extract parameters from the message
    std::vector<std::string> params = extractParams(message);

    // Check for sufficient parameters
    if (params.size() < 1) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters. Usage: MODE <channel> <+flag or -flag> [parameters]");
        return;
    }
	
	std::string channel_name = trim(params[0]);

	std::map<std::string, Channel>::iterator channel_it = channels.find(channel_name);
    if (channel_it == channels.end()) {
        sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; 

	if (params.size() == 1) {
        std::string current_modes = channel.getCurrentModeFlags();
        std::string mode_message = "324 " + clients[client_fd].getNickname() + " " + channel_name + " +"+ current_modes + "\r\n";
        send(client_fd, mode_message.c_str(), mode_message.length(), 0);
        return;
    }

	std::string	flags = trim(params[1]);
	std::string parameters = (params.size() > 2) ? trim(params[2]) : "";

	if (flags.empty() || (flags[0] != '+' && flags[0] != '-')) {
		sendError(client_fd, ERR_BADPARAM, "MODE", "Invalid mode flag. Usage: MODE <channel> <+flag or -flag> [parameters]");
		return;
	}
	flags = flags.substr(1);  // Trim the first character

	if (!channel.isOperator(client_fd)) {
        sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change modes.");
        return;
    }

	for (size_t i = 0; i < flags.length(); ++i) {
		char flag = flags[i];
		std::string mode_message;

		switch (flag) {
			case 'i':  // Invite-only mode
				channel.mode(flag, "");
				mode_message = channel_name + ": Invite-only mode " + (channel.getMode("inviteOnly") ? "enabled" : "disabled") + "\r\n";
				send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 't':  // Topic restriction mode
				channel.mode(flag, "");
				mode_message = channel_name + ": Topic restriction mode " + (channel.getMode("topicRestricted") ? "enabled" : "disabled") + "\r\n";
				send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 'k':  // Channel key
				if (parameters.empty() && !channel.getMode("keyReq")) {
					sendError(client_fd, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters to set the channel key.");
					return;
				}
					channel.mode(flag, parameters);
				if (channel.getMode("keyReq")) {
					mode_message = channel_name + ": Channel key set to '" + parameters + "'\r\n";
					send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				} else {
					mode_message = channel_name + ": Channel key requirement removed\r\n";
					send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				}
				break;

			case 'l':  // User limit
				if (!isNonNegativeInteger(parameters)) {
					sendError(client_fd, ERR_BADPARAM, "MODE", "User limit must be a valid non-negative integer.");
					return;
				}
				channel.mode(flag, parameters);
				mode_message = channel_name + ": User limit set to " + parameters + "\r\n";
				send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 'o': {  // Operator privilege
				if (params.size() < 3) {
					sendError(client_fd, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters to set operator privilege");
					return;
				}

				std::string target_nickname = trim(parameters); // Assuming parameters contains the nickname

				// Find the client fd associated with the nickname
				int target_fd = -1;
				for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
					int op_fd = it->first;
					// Check if the client's nickname matches the target nickname
					if (clients[op_fd].getNickname() == target_nickname) {
						target_fd = op_fd;
						break; // Stop searching once the target is found
					}
				}
				// If no client with the target nickname was found, send an error
				if (target_fd == -1) {
					sendError(client_fd, ERR_NOSUCHNICK, target_nickname, "No such nickname.");
					return;
				}
				// Check if the target_fd is in the channel
				const std::vector<int>& members = channel.getMembers();
				if (std::find(members.begin(), members.end(), target_fd) != members.end()) {
					std::stringstream ss;
					ss << target_fd;
					std::string target_fd_str = ss.str();
					channel.mode(flag, target_fd_str);
					if (channel.isOperator(target_fd)) {
						std::string mode_message = "MODE: Client " + target_nickname + " is now an operator.\r\n";
						send(client_fd, mode_message.c_str(), mode_message.length(), 0);
					} else {
						std::string mode_message = "MODE: Client " + target_nickname + " is no longer an operator.\r\n";
						send(client_fd, mode_message.c_str(), mode_message.length(), 0);
					}
				} else {
					sendError(client_fd, ERR_USERNOTINCHANNEL, "MODE", "No such client in channel.");
				}
				break;
			}
			default:
				sendError(client_fd, ERR_BADPARAM, "MODE", "Unknown mode flag.");
				return;
		}
	}
}
