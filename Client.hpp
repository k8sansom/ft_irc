#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>

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

    bool isValidNickname(const std::string& nickname);

private:
    int fd;
    std::string nickname;
    std::string username;
    std::string password;
    bool authenticated;
};

#endif
