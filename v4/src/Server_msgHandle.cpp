#include "../inc/Server.hpp"

void Server::handleClientMessage(int client_fd) {
	if (clients.find(client_fd) == clients.end()) {
		std::cerr << "ERROR: Client not found in the list" << std::endl;
		return;
	}

	std::string& buffer = clients[client_fd].getBuffer();
	char recv_buffer[512];
	ssize_t bytes_received = recv(client_fd, recv_buffer, sizeof(recv_buffer) - 1, 0);

	if (bytes_received == 0) {
		std::cout << "Client " << client_fd << " disconnected" << std::endl;
		close(client_fd);
		clients.erase(client_fd);
		return;
	} else if (bytes_received < 0) {
		std::cerr << "ERROR: Failed to receive data from client " << client_fd << std::endl;
		return;
	}

	recv_buffer[bytes_received] = '\0';
	buffer.append(recv_buffer);

	size_t pos;
	while ((pos = buffer.find('\n')) != std::string::npos) {
		std::string message = buffer.substr(0, pos);
		buffer.erase(0, pos + 1);

		if (!message.empty() && message[message.size() - 1] == '\r') {
			message.erase(message.size() - 1);  // Remove the last character
		}

		if (!processClientMessage(client_fd, message)) {
			std::cerr << "Error processing message from client " << client_fd << std::endl;
			return;
		}
	}
}

bool Server::processClientMessage(int client_fd, const std::string& message) {

	if (message.empty()) {
		return true;
	}

	std::cout << "Received from client " << client_fd << ": " << message << std::endl;

	if (message.rfind("CAP", 0) == 0) {
		std::cout << "CAP command received" << std::endl;
		return true;
	}

	if (!clients[client_fd].isAuthenticated() && message.rfind("PASS", 0) != 0) {
		std::string auth_response = "ERROR: You need to authenticate first!\r\n";
		send(client_fd, auth_response.c_str(), auth_response.length(), 0);
		close(client_fd);
		clients.erase(client_fd);
		std::cout << "Client " << client_fd << " disconnected due to lack of authentication" << std::endl;
		return false;
	}

	if (message.rfind("PASS", 0) == 0) {
		handlePassCommand(client_fd, message);
	} else if (message.rfind("NICK", 0) == 0) {
		handleNickCommand(client_fd, message);
	} else if (message.rfind("USER", 0) == 0) {
		handleUserCommand(client_fd, message);
	} else if (message.rfind("JOIN", 0) == 0) {
		handleJoinCommand(client_fd, message);
	} else if (message.rfind("PRIVMSG", 0) == 0) {
		handlePrivMsgCommand(client_fd, message);
	} else if (message.rfind("KICK", 0) == 0) {
		handleKickCommand(client_fd, message);
	} else if (message.rfind("INVITE", 0) == 0) {
		handleInviteCommand(client_fd, message);
	} else if (message.rfind("MODE", 0) == 0) {
		handleModeCommand(client_fd, message);
	} else if (message.rfind("TOPIC", 0) == 0) {
		handleTopicCommand(client_fd, message);
	} else if (message.rfind("WHO", 0) == 0) {
		handleWhoCommand(client_fd, message);
	} else {
		std::string wrong_command = "ERROR: Unknown command\r\n";
		send(client_fd, wrong_command.c_str(), wrong_command.length(), 0);
	}

	return true;
}
