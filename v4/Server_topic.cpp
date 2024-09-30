#include "Server.hpp"
#include "Channel.hpp"
#include "Client.hpp"

void Server::handleTopicCommand(int client_fd, const std::string& message) {
    std::string error_msg;
    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    // Extract parameters from the message
    std::vector<std::string> params = extractParams(message);

    // Check for sufficient parameters
    if (params.size() < 1) {
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
			if (!channel.isOperator(client_fd)) {
        		sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change topic.");
        		return;
    		}
	}

	// Perform the invite (this could be a function in your Channel class)
    channel.topic(new_topic);

    // Optionally, send a confirmation to the client who changed the topic
    std::string confirmation_message = channel_name + ": You changed the topic to \"" + new_topic + "\"\r\n";
    send(client_fd, confirmation_message.c_str(), confirmation_message.length(), 0);
}
