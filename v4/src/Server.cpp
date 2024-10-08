#include "../inc/Server.hpp"

std::map<int, Client> Server::clients;

int Server::server_socket = -1;

Server::Server(int port, const std::string& password): port(port), password(password) {
	setupSocket();
	bindSocket();
	listenSocket();
	serverName = "3,5 server";


	#ifdef BONUS
	std::cout << "Initializing bot..." << std::endl;
	bot = new Bot("server_bot");

	int bot_fd = -1; // Negative FD to indicate a pseudo-client
	bot->setBotFd(bot_fd);
	clients[bot_fd] = Client();
	clients[bot_fd].setNickname("server_bot");
	clients[bot_fd].setUsername("bot_username");
	clients[bot_fd].setRealname("Bot Realname");
	clients[bot_fd].authenticate();

	std::cout << "Bot manually added to clients map with FD: " << bot_fd << std::endl;

	#endif
}

Server::~Server() {}

std::map<int, Client>& Server::getClients() {
	return clients;
}

std::map<std::string, Channel>& Server::getChannels() {
	return channels;
}

std::vector<std::string> Server::receiveMessage(int client_fd) {
	char buffer[1024];
	int bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes_received < 0) {
		std::cerr << "ERROR: Failed to receive message from client " << client_fd << std::endl;
		return std::vector<std::string>();
	} else if (bytes_received == 0) {
		close(client_fd);
		clients.erase(client_fd);
		std::cout << "Client " << client_fd << " disconnected" << std::endl;
		return std::vector<std::string>();
	}

	buffer[bytes_received] = '\0';
	std::string data(buffer);
	std::vector<std::string> messages;
	size_t pos = 0;

	while ((pos = data.find("\r\n")) != std::string::npos) {
		messages.push_back(data.substr(0, pos));
		data.erase(0, pos + 2);
	}
	if (!data.empty()) {
		messages.push_back(data);
		data.clear();
	}

	return messages;
}

void Server::sendMessageToClient(int client_fd, const std::string& message) {
	ssize_t bytes_sent = send(client_fd, message.c_str(), message.length(), 0);

	if (bytes_sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		clients[client_fd].queueMessage(message);
	}
}

void Server::processQueuedMessages(int client_fd) {
	Client& client = clients[client_fd];

	while (client.hasQueuedMessages()) {
		std::string message = client.dequeueMessage();
		ssize_t bytes_sent = send(client_fd, message.c_str(), message.length(), 0);

		if (bytes_sent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			client.queueMessage(message);
			break;
		}
	}
}

void Server::floodControl(int client_fd) {
	if (clients[client_fd].getMessageQueueSize() > 30) {
		std::string err_msg = "ERROR: You are flooding the server.\r\n";
		send(client_fd, err_msg.c_str(), err_msg.length(), 0);
		close(client_fd);
		clients.erase(client_fd);
		std::cerr << "Client " << client_fd << " kicked for flooding." << std::endl;
	}
}
