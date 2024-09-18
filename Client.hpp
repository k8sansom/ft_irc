#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <ctime>
#include <sstream> 

class Client {
public:
    Client();
    Client(int fd, std::string nickname = "");
    ~Client();

    std::string getNickname() const;
    void setNickname(const std::string& nickname);
    int getFd() const;

    void setPassword(const std::string& password);
    bool checkPassword(const std::string& inputPassword) const;

    void setUsername(const std::string& username);
    std::string getUsername() const;

    void authenticate();
    bool isAuthenticated() const;

    bool isValidNickname(const std::string& nickname) const;
    bool isUniqueNickname(const std::string& nickname, const std::map<int, Client>& clients) const;
    std::string sanitizeNickname(const std::string& nickname) const;
    std::string getUniqueNickname(const std::string& nickname, const std::map<int, Client>& clients) const;


private:
    int 		_fd;
    std::string _nickname;
    std::string _username;
    std::string _password;
    bool 		_authenticated;
	// bool		_operator;
};

#endif
