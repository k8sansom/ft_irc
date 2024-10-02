#ifdef BONUS
#include "../inc/Server.hpp"
    
void Server::handleDccSendRequest(int client_fd, const std::string& message) {

    // Extract details from the DCC SEND message
    std::istringstream iss(message);
    std::string command, filename, ip_str, port_str, filesize_str;
    iss >> command >> filename >> ip_str >> port_str >> filesize_str;

    // Debug: Print out the extracted fields
    std::cout << "Command: " << command << ", Filename: " << filename << ", IP: " << ip_str << ", Port: " << port_str << ", Filesize: " << filesize_str << std::endl;

    if (command != "DCC" || filename.empty() || ip_str.empty() || port_str.empty() || filesize_str.empty()) {
        std::string errorMsg = "ERROR :Invalid DCC SEND command format\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Convert IP address and port to the appropriate types
    unsigned long ip = inet_addr(ip_str.c_str());
    int port = std::atoi(port_str.c_str());
    size_t filesize = std::atoi(filesize_str.c_str());

    if (ip == INADDR_NONE || port <= 0 || filesize <= 0) {
        std::string errorMsg = "ERROR :Invalid IP/Port/File size in DCC SEND command\r\n";
        send(client_fd, errorMsg.c_str(), errorMsg.length(), 0);
        return;
    }

    // Debug: Print out converted values
    std::cout << "Converted IP: " << ip << ", Port: " << port << ", Filesize: " << filesize << std::endl;

    // Create a socket for file transfer
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        std::cerr << "Failed to create socket" << std::endl;
        return;
    }

    struct sockaddr_in receiver_addr;
    memset(&receiver_addr, 0, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_port = htons(port);
    receiver_addr.sin_addr.s_addr = ip;

    // Debug: Print receiver info
    std::cout << "Attempting to connect to IP: " << inet_ntoa(receiver_addr.sin_addr) << " on port: " << ntohs(receiver_addr.sin_port) << std::endl;

    // Connect to the receiver
    if (connect(sock, (struct sockaddr*)&receiver_addr, sizeof(receiver_addr)) < 0) {
        std::cerr << "Failed to connect to receiver" << std::endl;
        close(sock);
        return;
    }

    // Open the file to send
    std::ifstream file(filename.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << " (error: " << strerror(errno) << ")" << std::endl;
        close(sock);
        return;
    }

    // Debug: Print file opened successfully
    std::cout << "File opened successfully: " << filename << std::endl;

    // Send the file in chunks
    char buffer[4096];
    size_t bytes_sent = 0;
    while (bytes_sent < filesize && file.good()) {
        file.read(buffer, sizeof(buffer));
        size_t bytes_read = file.gcount();

        if (send(sock, buffer, bytes_read, 0) < 0) {
            std::cerr << "Failed to send data" << std::endl;
            break;
        }
        bytes_sent += bytes_read;

        // Debug: Print file sending progress
        std::cout << "Bytes sent: " << bytes_sent << " / " << filesize << std::endl;
    }

    if (bytes_sent == filesize) {
        std::cout << "File transfer completed successfully: " << filename << std::endl;
    } else {
        std::cerr << "File transfer failed" << std::endl;
    }

    // Close the file and socket
    file.close();
    close(sock);
}
#endif
