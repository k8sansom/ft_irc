#include <string>
#include <vector>
#include <algorithm>

class Channel {
private:
    std::string 		_name;
    std::vector<int>	_members;  // List of client FDs
	int					_operator_fd;

public:
    Channel(const std::string& channelName);
	~Channel();
    const std::string& getName() const;
    bool addClient(int client_fd);
	// Remove a client from the channel
    void removeClient(int client_fd);
	// Check if the channel is empty
    bool isEmpty() const;
	// make client operator

    // Broadcast a message to all members except the sender
    void broadcastMessage(const std::string& message, int sender_fd);
};