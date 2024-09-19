#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <iostream>

class Channel {
private:
    std::string 		_name;
    std::vector<int>	_members;  // List of client FDs
	int					_operator_fd;

public:
    Channel();
    Channel(const std::string& channelName, int operator_fd);
	~Channel();
    const std::string& getName() const;
    const std::vector<int>& getMembers() const;

    bool addClient(int client_fd);
    void removeClient(int client_fd);
    bool isEmpty() const;
    bool isOperator(int client_fd) const;
    void broadcastMessage(const std::string& message, int sender_fd);

};

#endif