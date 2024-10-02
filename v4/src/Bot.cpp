#ifdef BONUS
#include "../inc/Bot.hpp"
#include "../inc/Server.hpp"

Bot::Bot(const std::string& nickname): nickname(nickname), bot_fd(-1) {
	jokes.push_back("I was going to tell a sodium joke, then I thought, \"Na\"");
    jokes.push_back("Why do ducks have feathers on their tales? To hide their butt-quacks.");
    jokes.push_back("What do you call it when a cow grows facial hair? A moo-stache.");

}

Bot::~Bot() {}

int Bot::getBotFd() const {
    return bot_fd;
}

void Bot::setBotFd(int fd) {
    this->bot_fd = fd;
}


void Bot::connectToServer(int port, const std::string& password) {
    bot_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (bot_fd < 0) {
        std::cerr << "Bot: Failed to create socket" << std::endl;
        return;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(bot_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Bot: Failed to connect to server" << std::endl;
        return;
    }

    std::cout << "Bot connected to server successfully!" << std::endl;

    sendCommand("PASS " + password);
    sendCommand("NICK " + nickname);
    sendCommand("USER " + nickname + " 0 * :" + nickname);
}

void Bot::joinChannel(const std::string& channel_name) {
    sendCommand("JOIN " + channel_name);
}

void Bot::handleMessage(const std::string& message, int client_fd, const std::string& channel_name, Server* server) {
	std::string lower_message = toLower(message); 

    if (lower_message.rfind("ping", 0) == 0) {
        std::string pong_response = "PONG :" + message.substr(5);
        sendCommand(pong_response);
        std::cout << "Responding to PING with PONG" << std::endl;
        return;
    }


    if (lower_message == "!help") {
        sendHelpMessage(client_fd, channel_name, server);
    } else if (lower_message == "!users") {
        sendUserList(client_fd, channel_name, server);
    } else if (lower_message == "!joke") {
        sendJoke(client_fd, channel_name, server);
    } else {
        std::string errMsg = "Unknown bot command.\r\n";
        server->sendBotMessage(client_fd, getBotFd(), channel_name, errMsg);
    }
}

void Bot::sendCommand(const std::string& command) {
    std::string msg = command + "\r\n";
    send(bot_fd, msg.c_str(), msg.length(), 0);
}

void Bot::sendMessage(const std::string& target, const std::string& message) {
    sendCommand("PRIVMSG " + target + " :" + message);
}

void Bot::sendHelpMessage(int client_fd, const std::string& channel_name, Server* server) {
    std::string helpMsg = "Bot commands: !help, !users, !joke";
    server->sendBotMessage(client_fd, getBotFd(), channel_name, helpMsg);
}

void Bot::sendUserList(int client_fd, const std::string& channel_name, Server* server) {
    std::string userList = "Users in the server: ";
    std::map<int, Client>& clients = server->getClients();
    for (std::map<int, Client>::iterator it = clients.begin(); it != clients.end(); ++it) {
        userList += it->second.getNickname() + " ";
    }
    server->sendBotMessage(client_fd, getBotFd(), channel_name, userList);
}

void Bot::sendJoke(int client_fd, const std::string& channel_name, Server* server) {
    if (jokes.empty())
        return;
    size_t jokeIndex = std::rand() % jokes.size();
    std::string jokeMsg = "Here's a joke: " + jokes[jokeIndex];
    server->sendBotMessage(client_fd, getBotFd(), channel_name, jokeMsg);
}

bool Bot::detectShit(const std::string& message, int client_fd, const std::string& channel_name, Server* server) {
    std::vector<std::string> inappropriate_words;
    std::string lower_message = toLower(message); 
	inappropriate_words.push_back("fuck");
	inappropriate_words.push_back("shit");
    inappropriate_words.push_back("motherfucker");
    inappropriate_words.push_back("cunt");
    inappropriate_words.push_back("bullshit");
    inappropriate_words.push_back("asshole");
    inappropriate_words.push_back("pidoras");
    for (std::vector<std::string>::iterator it = inappropriate_words.begin(); it != inappropriate_words.end(); ++it) {
        if (lower_message.find(*it) != std::string::npos) {
            std::string warning = "Warning! Please avoid using inappropriate language.";
            server->sendBotMessage(client_fd, getBotFd(), channel_name, warning);
            return true;
        }
    }
    return false;
}

std::string Bot::toLower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

#endif