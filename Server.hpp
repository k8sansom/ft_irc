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

#include "Client.hpp"

class Server {
public:
    Server(int port, const std::string& password);
    ~Server();

    std::map<int, Client>& getClients();
    std::map<std::string, std::vector<int> >& getChannels();
    void pollClients();
    
private:
    int server_socket;
    int port;
    std::string password;
    sockaddr_in server_address;

    std::map<int, Client> clients;
    std::map<std::string, std::vector<int> > channels;

    void setupSocket();
    void bindSocket();
    void listenSocket();
    void acceptClient();
    void handleClientMessage(int client_fd);
    bool checkName(std::string nickname);
};

#endif
