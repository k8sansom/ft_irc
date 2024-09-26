#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <sys/types.h>
#include <sys/socket.h>
#include "Client.hpp"

class Channel {
private:
    std::string _name;
    std::vector<int> _members;
    std::string _topic;
    std::string _key; 
    std::set<int> _invitedClients;
    std::set<int> _operators;
	bool _inviteOnly;
	bool _topicRestricted;
	bool _keyReq;
	unsigned long _userLimit;


public:
    Channel();
    Channel(const std::string& channelName, int operator_fd);
    ~Channel();

    // Getters
    const std::string& getName() const;
    const std::vector<int>& getMembers() const;
    std::string getTopic() const;
	bool getMode(const std::string mode) const;

    bool addClient(int client_fd);
    void removeClient(int client_fd);
    bool isEmpty() const;
    bool isOperator(int client_fd) const;
    void broadcastMessage(const std::string& message, int sender_fd);
    bool checkChannelKey(const std::string key) const;
	bool checkInvite(int client_fd) const;
	bool checkUserLimit(void) const;


	//operator commands
    void kick(Client& targetClient, const std::string& reason);
    void invite(Client& targetClient);
	void topic(const std::string& newTopic);
	void mode(const char flag, const std::string& param);
};

#endif