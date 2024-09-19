#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <utility>

#include "Client.hpp"
#include "Channel.hpp"

bool isValidPassword(const std::string& password);

class Server {
public:
    Server(int port, const std::string& password);
    ~Server();

    std::map<int, Client>& getClients();
    std::map<std::string, Channel>& getChannels();
    void pollClients();
    
private:
    int server_socket;
    int port;
    std::string password;
    sockaddr_in server_address;
    std::map<std::string, Channel> channels;
    std::map<int, Client> clients;

    // Socket setup and management
    void setupSocket();
    void bindSocket();
    void listenSocket();
    void acceptClient();

    // Client message handling
    void handleClientMessage(int client_fd);
    std::vector<std::string> receiveMessage(int client_fd);

    // Command handling
    void handlePassCommand(int client_fd, const std::string& message);
    void handleNickCommand(int client_fd, const std::string& message);
    void handleUserCommand(int client_fd, const std::string& message);
    void handleJoinCommand(int client_fd, const std::string& message);
    void handlePrivMsgCommand(int client_fd, const std::string& message);

};

#endif
