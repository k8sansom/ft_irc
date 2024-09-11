#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>
#include <map>
#include <vector>
#include <stdexcept>

class Client {
public:
	Client(int fd);
	~Client();

	std::string getNickname() const;
	void setNickname(const std::string& nickname);
	int getFd() const;

	void authentificate();
	bool isAuthenticated() const;

	bool isValidNickname(const std::string& nickname);

	bool isOperator();
	
private:
	int fd;
	std::string nickname;
};

class Server {
public:
	Server(int port, const std::string& password);
	~Server();


	std::map<int, Client>& getClients();
    std::map<std::string, std::vector<int>>& getChannels();

	bool checkName(std::string nickname);

private:
	int server_socket;
	int port;
	std::string password;
	sockaddr_in server_address;

	std::map<int, Client> clients; // client fd as the key + Client obj
	std::map<std::string, std::vector<int>> channels; // name of the channel + vector of client fds connected to it

	void setupSocket();
    void bindSocket();
    void listenSocket();
};

#endif