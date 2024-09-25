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
    if (params.size() < 2) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters. Usage: MODE <channel> <+flag or -flag> [parameters]");
        return;
    }

    std::string channel_name = trim(params[0]);
	std::string	flags = trim(params[1]);
	std::string parameters = (params.size() > 2) ? trim(params[2]) : "";

	if (flags.empty() || (flags[0] != '+' && flags[0] != '-')) {
		sendError(client_fd, ERR_BADPARAM, "MODE", "Invalid mode flag. Usage: MODE <channel> <+flag or -flag> [parameters]");
		return;
	}
	flags = flags.substr(1);  // Trim the first character

    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_name);
    if (channel_it == channels.end()) {
        sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; // Get the channel object

	if (!channel.isOperator(client_fd)) {
        sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change modes.");
        return;
    }

	for (size_t i = 0; i < flags.length(); ++i) {
		char flag = flags[i];
		std::string mode_message;

		switch (flag) {
			case 'i':  // Invite-only mode
				channel.setMode(clients[client_fd], flag, "");
				mode_message = channel_name + ": Invite-only mode " + (channel.getMode("inviteOnly") ? "enabled" : "disabled") + "\r\n";
				send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 't':  // Topic restriction mode
				channel.setMode(clients[client_fd], flag, "");
				mode_message = channel_name + ": Topic restriction mode " + (channel.getMode("topicRestricted") ? "enabled" : "disabled") + "\r\n";
				send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 'k':  // Channel key
				if (parameters.empty()) {
					sendError(client_fd, ERR_NEEDMOREPARAMS, "MODE", "Not enough parameters to set the channel key.");
					return;
				}
				channel.setMode(clients[client_fd], flag, parameters);
				mode_message = channel_name + ": Channel key set to '" + parameters + "'\r\n";
				send(client_fd, mode_message.c_str(), mode_message.length(), 0);
				break;

			case 'l':  // User limit
				if (!isNonNegativeInteger(parameters)) {
					sendError(client_fd, ERR_BADPARAM, "MODE", "User limit must be a valid non-negative integer.");
					return;
				}
				channel.setMode(clients[client_fd], flag, parameters);
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
					channel.setMode(clients[client_fd], flag, parameters);
					std::string mode_message = "MODE: Client " + target_nickname + " is now an operator.\r\n";
					send(client_fd, mode_message.c_str(), mode_message.length(), 0);
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

void Server::handleTopicCommand(int client_fd, const std::string& message) {
    std::string error_msg;
    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Extract parameters from the message
    std::vector<std::string> params = extractParams(message);

    // Check for sufficient parameters
    if (params.size() < 2) {
        sendError(client_fd, ERR_NEEDMOREPARAMS, "TOPIC", "Not enough parameters.");
        return;
    }

    std::string channel_name = trim(params[0]);

    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_name);
    if (channel_it == channels.end()) {
        sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; // Get the channel object

	if (params.size() < 2) {
        std::string current_topic = channel.getTopic();
        if (current_topic.empty()) {
            send(client_fd, "No topic is set for this channel.\r\n", 34, 0);
        } else {
            std::string topic_message = "Current topic for " + channel_name + ": " + current_topic + "\r\n";
            send(client_fd, topic_message.c_str(), topic_message.length(), 0);
        }
        return;
    }

	std::string new_topic;
    for (size_t i = 1; i < params.size(); ++i) {
        if (i > 1) {
            new_topic += " ";  // Add a space between parameters
        }
        new_topic += params[i];
    }
    new_topic = trim(new_topic);

	if (channel.getMode("topicRestricted")) {
			std::cout <<"this is coming back true?!" <<std::endl;
			if (!channel.isOperator(client_fd)) {
        		sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change topic.");
        		return;
    		}
	}

	// Perform the invite (this could be a function in your Channel class)
    channel.setTopic(clients[client_fd], new_topic);

    // Optionally, send a confirmation to the client who changed the topic
    std::string confirmation_message = channel_name + ": You changed the topic to \"" + new_topic + "\"\r\n";
    send(client_fd, confirmation_message.c_str(), confirmation_message.length(), 0);
}

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
        return;
    }

    std::string channel_name = trim(params[0]);
    std::string target_nickname = trim(params[1]);

    std::map<std::string, Channel>::iterator channel_it = channels.find(channel_name);
    if (channel_it == channels.end()) {
        sendError(client_fd, ERR_NOSUCHCHANNEL, channel_name, "No such channel.");
        return;
    }

    Channel& channel = channel_it->second; // Get the channel object

	if (!channel.isOperator(client_fd)) {
        sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to invite.");
        return;
    }

    int target_fd = -1;
	for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
		int invite_fd = it->first;
		
		// Check if the client's nickname matches the target nickname
		if (clients[invite_fd].getNickname() == target_nickname) {
			target_fd = invite_fd;
			break; // Stop searching once the target is found
		}
	}

	// If no client with the target nickname was found, send an error
	if (target_fd == -1) {
		sendError(client_fd, ERR_NOSUCHNICK, target_nickname, "No such nickname.");
		return;
	}

    // Check if the target client is already in the channel
	const std::vector<int>& members = channel.getMembers();
	bool isAlreadyMember = false;

	for (std::vector<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
    	if (*it == target_fd) {
        	isAlreadyMember = true;
        	break;
    	}
	}
	if (isAlreadyMember) {
		std::string already_msg = channel_name + ": " + target_nickname + " is already in the channel\r\n";
		send(client_fd, already_msg.c_str(), already_msg.length(), 0);
		return;
	}
	// Perform the invite (this could be a function in your Channel class)
    channel.invite(clients[client_fd], clients[target_fd]);

	// Notify the target client about the invite
    std::string invite_message = clients[client_fd].getNickname() + " invited you to " + channel_name + "\r\n";
    send(target_fd, invite_message.c_str(), invite_message.length(), 0);

    // Optionally, send a confirmation to the client who sent the invite
    std::string confirmation_message = "You invited " + target_nickname + " to " + channel_name + "\r\n";
    send(client_fd, confirmation_message.c_str(), confirmation_message.length(), 0);
}

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

    std::string channel_name = trim(params[0]);
    std::string target_nickname = trim(params[1]);
	std::string reason;
	if (params.size() > 2) {
    	for (size_t i = 1; i < params.size(); ++i) {
        	if (i > 1) {
            	reason += " ";  // Add a space between parameters
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
        sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to kick.");
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
        sendError(client_fd, ERR_NOSUCHNICK, target_nickname, "No such nickname.");
        return;
    }

    // // Perform the kick
    channel.kick(clients[client_fd], clients[target_fd], reason);

    // Notify the target about the kick
    std::string kick_message = channel_name + ": " + clients[client_fd].getNickname() + " kicked you out of " + channel_name + "\r\n";
    send(target_fd, kick_message.c_str(), kick_message.length(), 0);
}
