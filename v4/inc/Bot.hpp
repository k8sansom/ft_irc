#ifdef BONUS
#ifndef BOT_HPP
#define BOT_HPP

#include "lib_inc.hpp"
#include "Server.hpp"

class Server;

class Bot {
public:
    Bot(const std::string& nickname);
    ~Bot();

    void connectToServer(int port, const std::string& password);
    void joinChannel(const std::string& channel_name);
    void handleMessage(const std::string& message, int client_fd, const std::string& channel_name, Server* server);
    int getBotFd() const;
    void setBotFd(int fd);
    bool detectShit(const std::string& message, int client_fd, const std::string& channel_name, Server* server);

private:
    std::string nickname;
    int bot_fd;

	void sendCommand(const std::string& command);
	void sendMessage(const std::string& target, const std::string& message);
    void sendHelpMessage(int client_fd, const std::string& channel_name,Server* server);
    void sendUserList(int client_fd, const std::string& channel_name, Server* server);
    void sendJoke(int client_fd, const std::string& channel_name, Server* server);

	std::string toLower(const std::string& str);

	std::vector<std::string> jokes;

};

#endif
#endif
