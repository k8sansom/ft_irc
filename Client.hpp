#ifndef CLIENT_HPP
#define CLIENT_HPP

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
#include <stdexcept>

class Client {
public:
	Client(int fd, std::string nickname, bool authentificated);
	~Client();

	std::string getNickname() const;
	void setNickname(const std::string& nickname);
	int getFd() const;

	void authentificate();
	bool isAuthenticated() const;

	bool isValidNickname(const std::string& nickname);

private:
	int fd;
	std::string nickname;
    bool authentificated;
    
};

#endif