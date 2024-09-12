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

    void authentificate();
    bool isAuthenticated() const;

    // Declare it as a member function
    bool isValidNickname(const std::string& nickname);

private:
    int fd;
    std::string nickname;
    bool authentificated;
};

#endif
