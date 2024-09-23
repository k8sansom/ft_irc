#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <sys/socket.h>
#include <iostream>

class Channel {
private:
    std::string         _name;
    std::vector<int>    _members;
    int                 _operator_fd;
    std::string         _topic;
    std::string         _key; 

public:
    Channel();
    Channel(const std::string& channelName, int operator_fd);
    Channel(const std::string& channelName, int operator_fd, const std::string& key); 
    ~Channel();

    const std::string& getName() const;
    const std::vector<int>& getMembers() const;
    const std::string& getTopic() const;
    void setTopic(const std::string& topic);

    bool addClient(int client_fd);
    void removeClient(int client_fd);
    bool isEmpty() const;
    bool isOperator(int client_fd) const;
    void broadcastMessage(const std::string& message, int sender_fd);
    void setKey(const std::string& key);
    bool canClientJoin(const std::string& key) const;
};

#endif