#include "../inc/Server.hpp"


void Server::handlePassCommand(int client_fd, const std::string& message) {
    if (!isCommandFormatValid(message, "PASS")) {
        std::string err_msg = "USAGE: PASS <password>\r\n";
        send(client_fd, err_msg.c_str(), err_msg.length(), 0);
        return;
    }
    std::string pass = message.substr(5);
    pass.erase(pass.find_last_not_of("\r\n") + 1);

    if (pass == this->password) {
        std::cout << "Client " << client_fd << " provided correct password." << std::endl;
        std::string corr_pass_response = "Password correct\r\n";
        send(client_fd, corr_pass_response.c_str(), corr_pass_response.length(), 0);
        clients[client_fd].authenticate();
        std::string auth_response = "You are successfully authenticated\r\n";
        send(client_fd, auth_response.c_str(), auth_response.length(), 0);
    } else {
        std::string incorr_pass_response = "ERROR :Invalid password\r\n";
        send(client_fd, incorr_pass_response.c_str(), incorr_pass_response.length(), 0);
    }
}

void Server::handleWhoCommand(int client_fd, const std::string& message) {
    std::vector<std::string> params = extractParams(message);
    if (params.empty()) {
        std::string errorMsg = "461 * ERR_NEEDMOREPARAMS :Not enough parameters for WHO\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    std::string target = params[0];

    if (channels.find(target) != channels.end()) {
        Channel& channel = channels[target];
        const std::vector<int>& members = channel.getMembers();

        for (std::vector<int>::const_iterator it = members.begin(); it != members.end(); ++it) {
            int member_fd = *it;
            Client& member = clients[member_fd];

            std::string who_reply = "352 " + target + " " + member.getNickname() + " " + member.getUsername() +
                                    " " + member.getRealName() + " :H\r\n";  // Simplified response
            send(client_fd, who_reply.c_str(), who_reply.length(), 0);
        }

        std::string end_of_who = "315 " + target + " :End of /WHO list\r\n";
        send(client_fd, end_of_who.c_str(), end_of_who.length(), 0);
    } else {
        std::string errorMsg = "403 " + target + " :No such channel\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
    }
}

void Server::handlePartCommand(int client_fd, const std::string& message) {
    if (clients.find(client_fd) == clients.end()) {
        std::cerr << "ERROR: Client not found in the list" << std::endl;
        return;
    }

    std::vector<std::string> params = extractParams(message);
    if (params.empty()) {
        sendInfoMessage(client_fd, 461, "*", "Not enough parameters for PART");
        return;
    }

    std::string channel_name = params[0];

    if (channels.find(channel_name) == channels.end()) {
        sendInfoMessage(client_fd, 403, channel_name, "No such channel");
        return;
    }

    Channel& channel = channels[channel_name];

    const std::vector<int>& members = channel.getMembers();
    if (std::find(members.begin(), members.end(), client_fd) == members.end()) {
        sendInfoMessage(client_fd, 442, channel_name, "You're not on that channel");
        return;
    }

    std::string partMessage = clients[client_fd].getNickname() + " you left the channel.";
    sendInfoMessage(client_fd, 0, channel_name, partMessage);

    std::string broadcastMessage = ":" + clients[client_fd].getNickname() + " PART " + channel_name + "\r\n";
    channel.broadcastMessage(broadcastMessage, client_fd);

    channel.removeClient(client_fd);

    if (channel.getMembers().empty()) {
        channels.erase(channel_name);
        std::cout << "Channel " << channel_name << " has been deleted as it is now empty." << std::endl;
    }
}
