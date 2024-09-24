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
    int _operator_fd;
    std::string _topic;
    std::string _key; 
    std::set<int> _invitedClients;
    std::set<int> _operators;
	bool _inviteOnly;
	bool _topicRestricted;
	bool _keyReq;
	int _userLimit;


public:
    Channel();
    Channel(const std::string& channelName, int operator_fd);
    Channel(const std::string& channelName, int operator_fd, const std::string& key); 
    ~Channel();

    // Getters
    const std::string& getName() const;
    const std::vector<int>& getMembers() const;
    std::string getTopic() const;
	bool getMode(const std::string mode) const;

    // Setters
    void setKey(const std::string& key);
    void setTopic(Client& operatorClient, const std::string& newTopic);
	// void setMode(Client& operatorClient, const std::string& modeFlags, const std::string& param);

    bool addClient(int client_fd);
    void removeClient(int client_fd);
    bool isEmpty() const;
    bool isOperator(int client_fd) const;
    void broadcastMessage(const std::string& message, int sender_fd);
    bool canClientJoin(const std::string& key) const;
    void kick(Client& operatorClient, Client& targetClient, const std::string& reason);
    void invite(Client& operatorClient, Client& targetClient);
};

#endif