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
#include <sstream> 

#include "ErrorCodes.hpp"

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

    // Server_connection
    void setupSocket();
    void bindSocket();
    void listenSocket();
    void acceptClient();

    // Client message handling
    void handleClientMessage(int client_fd);
    std::vector<std::string> receiveMessage(int client_fd);
    void handlePassCommand(int client_fd, const std::string& message);

    // Utils
    bool isCommandFormatValid(const std::string& message, const std::string& command);
    void sendError(int client_fd, int error_code, const std::string& target, const std::string& message);
    std::vector<std::string> split(const std::string& str, char delimiter);
	std::string trim(const std::string& str);

    //Server_nick
    bool validateFormat(int client_fd, const std::string& message);
    bool checkAlreadySet(int client_fd, const std::string& nick);
    bool sanitizeNickname(int client_fd, std::string& nick, std::string& err_msg);
    bool checkUnique(int client_fd, std::string& nick, std::string& err_msg);
    void setNickname(int client_fd, const std::string& nick);
    void handleNickCommand(int client_fd, const std::string& message);

    //Server_user
    void handleUserCommand(int client_fd, const std::string& message);
    std::vector<std::string> extractUserParams(const std::string& message);
    bool validateUserCommand(int client_fd, const std::vector<std::string>& params);
    std::string sanitizeUsername(const std::string& username);
    void sendUsernameConfirmation(int client_fd, const std::string& username, const std::string& realname);

    //Server_privmsg
    void handlePrivMsgCommand(int client_fd, const std::string& message);
    std::pair<std::string, std::string> extractTargetAndMsg(const std::string& message, int client_fd);
    void handleChannelMsg(int client_fd, const std::string& target, const std::string& msgContent);
    void handleUserMsg(int client_fd, const std::string& target, const std::string& msgContent);

    //Server_join
    void handleJoinCommand(int client_fd, const std::string& message);
    std::vector<std::string> extractParams(const std::string& message);
    void leaveAllChannels(int client_fd);
    bool isValidChannelName(const std::string& name);
    void joinExistingChannel(int client_fd, const std::string& channel_name, const std::string& key);
    void createAndJoinChannel(int client_fd, const std::string& channel_name, const std::string& key);
    void sendJoinConfirmation(int client_fd, const std::string& channel_name);
    void sendChannelInfo(int client_fd, const std::string& channel_name);

	//Server_opcmds
	void handleKickCommand(int client_fd, const std::string& message);
	void handleInviteCommand(int client_fd, const std::string& message);
	void handleTopicCommand(int client_fd, const std::string& message);
	void handleModeCommand(int client_fd, const std::string& message);
};

#endif
