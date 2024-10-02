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
        sendError(client_fd, ERR_NOSUCHCHANNEL, "MODE", "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; 

	if (params.size() == 1) {
        std::string current_modes = channel.getCurrentModeFlags();
        std::string mode_message = current_modes + "\r\n";
		sendInfoMessage(client_fd, RPL_CHANNELMODEIS, channel_name, mode_message);
        return;
    }

	std::string	flags = trim(params[1]);
	std::string parameters = (params.size() > 2) ? trim(params[2]) : "";

	if (flags.empty() || (flags[0] != '+' && flags[0] != '-')) {
		// sendError(client_fd, ERR_BADPARAM, "MODE", "Invalid mode flag. Usage: MODE <channel> <+flag or -flag> [parameters]");
		sendInfoMessage(client_fd, ERR_BADPARAM, channel_name, "Invalid mode flag. Usage: MODE <channel> <+flag or -flag> [parameters]");
		return;
	}
	char sign = flags[0];

	if (!channel.isOperator(client_fd)) {
        // sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change modes.");
		sendInfoMessage(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change modes.");
        return;
    }

	for (size_t i = 1; i < flags.length(); ++i) {
		char flag = flags[i];
		std::string mode_message;
		switch (flag) {
			case 'i':  // Invite-only mode
				channel.mode(flag, sign, "");
				if (sign == '+') {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " +i\r\n";
					// mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " +i " + parameters + "\r\n";
				} else {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " -i\r\n";
				}
				channel.broadcastMessage(mode_message, -1);
				// send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 't':  // Topic restriction mode
				channel.mode(flag, sign, "");
				if (sign == '+') {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " +t " + parameters + "\r\n";
				} else {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " -t\r\n";
				}
				channel.broadcastMessage(mode_message, -1);
				// send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 'k':  // Channel key
				if (parameters.empty() && sign == '+') {
					// sendError(client_fd, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters to set the channel key.");
					sendInfoMessage(client_fd, ERR_NEEDMOREPARAMS, channel_name, "Not enough parameters to set the channel key.");
					return;
				}
				channel.mode(flag, sign, parameters);
				if (sign == '+') {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " +k " + parameters + "\r\n";
				} else {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " -k\r\n";
				}
				channel.broadcastMessage(mode_message, -1);
				// send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 'l':  // User limit
				if (sign == '+' && !isNonNegativeInteger(parameters)) {
					// sendError(client_fd, ERR_BADPARAM, "MODE", "User limit must be a valid non-negative integer.");
					sendInfoMessage(client_fd, ERR_BADPARAM, channel_name, "User limit must be a valid non-negative integer.");
					return;
				}
				channel.mode(flag, sign, parameters);
				if (sign == '+') {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " +l " + parameters + "\r\n";
				} else {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " -l\r\n";
				}
    			channel.broadcastMessage(mode_message, -1);
				// send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 'o': {  // Operator privilege
				if (params.size() < 3) {
					// sendError(client_fd, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters to give or take operator privilege");
					sendInfoMessage(client_fd, ERR_NEEDMOREPARAMS, channel_name, "Not enough parameters to give or take operator privilege");
					return;
				}

				std::string target_nickname = trim(parameters);
				int target_fd = -1;
				for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
					int op_fd = it->first;
					if (clients[op_fd].getNickname() == target_nickname) {
						target_fd = op_fd;
						break;
					}
				}
				if (target_fd == -1) {
					// sendError(client_fd, ERR_NOSUCHNICK, target_nickname, "No such nickname.");
					sendInfoMessage(client_fd, ERR_NOSUCHNICK, channel_name, "No such nickname.");
					return;
				}
				const std::vector<int>& members = channel.getMembers();
				if (std::find(members.begin(), members.end(), target_fd) != members.end()) {
					std::stringstream ss;
    				ss << target_fd;
    				std::string target_str = ss.str();
					channel.mode(flag, sign, target_str);
				} else {
					// sendError(client_fd, ERR_USERNOTINCHANNEL, "MODE", "No such client in channel.");
					sendInfoMessage(client_fd, ERR_USERNOTINCHANNEL, channel_name, "No such client in channel.");
				}
				if (sign == '+') {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " +o " + target_nickname + "\r\n";
				} else {
					mode_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server MODE " + channel_name + " -o " + target_nickname + "\r\n";
				}
				channel.broadcastMessage(mode_message, -1);
				// send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;
			}
			default:
				// sendError(client_fd, ERR_BADPARAM, "MODE", "Unknown mode flag.");
				sendInfoMessage(client_fd, ERR_BADPARAM, channel_name, "Unknown mode flag.");
				return;
		}
	}
}
