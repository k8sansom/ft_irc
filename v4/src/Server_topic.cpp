#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Client.hpp"

void Server::handleTopicCommand(int client_fd, const std::string& message) {
    std::string error_msg;
    if (!clients[client_fd].checkAttributes(error_msg)) {
        send(client_fd, error_msg.c_str(), error_msg.length(), 0);
        return;
    }

    std::vector<std::string> params = extractParams(message);

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

    Channel& channel = channel_it->second;

    if (params.size() == 1) {
        std::string current_topic = channel.getTopic();
        if (current_topic.empty()) {
            sendInfoMessage(client_fd, RPL_NOTOPIC, channel_name, "No topic is set");
        } else {
            sendInfoMessage(client_fd, RPL_TOPIC, channel_name, current_topic);
        }
        return;
    }

    std::string new_topic = trim(params[1]);
    // Remove colon at the start if present
    if (new_topic[0] == ':') {
        new_topic = new_topic.substr(1);
    }

    if (new_topic.empty()) {
        channel.topic("");

        std::string broadcast_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server TOPIC " + channel_name + " :\r\n";
        channel.broadcastMessage(broadcast_message, -1);

        std::string confirmation_message = "You have cleared the topic for " + channel_name + "\r\n";
        send(client_fd, confirmation_message.c_str(), confirmation_message.length(), 0);

        return;
    }

    for (size_t i = 2; i < params.size(); ++i) {
        if (i > 1) {
            new_topic += " ";
        }
        new_topic += params[i];
    }

    new_topic = trim(new_topic);

	if (channel.getMode("topicRestricted")) {
        if (!channel.isOperator(client_fd)) {
            sendError(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change topic.");
            sendInfoMessage(client_fd, ERR_CHANOPRIVSNEEDED, channel_name, "You do not have permission to change the topic.");
            return;
        }
	}

    channel.topic(new_topic);

    std::string broadcast_message = ":" + clients[client_fd].getNickname() + "!" + clients[client_fd].getUsername() + "@server TOPIC " + channel_name + " :" + new_topic + "\r\n";
    channel.broadcastMessage(broadcast_message, -1);

    std::string confirmation_message = "You changed the topic to: \"" + new_topic + "\"\r\n";
    send(client_fd, confirmation_message.c_str(), confirmation_message.length(), 0);
}