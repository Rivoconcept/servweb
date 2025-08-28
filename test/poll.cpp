#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <string.h>

#define LISTENNING_PORT 8080
#define MAX_PENDING_QUEUE 10
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 100

int main()
{
    // 1. Create the socket
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFd == -1)
    {
        std::cerr << "Error: socket initialization failed!!!" << std::endl;
        return 1;
    }

    // Enable SO_REUSEADDR to reuse the address immediately
    int opt = 1;
    if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Error: setsockopt failed!!!" << std::endl;
        close(socketFd);
        return 1;
    }

    // 2. Bind the IP and port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(LISTENNING_PORT);

    int socketAddressLength = sizeof(address);
    if (bind(socketFd, (sockaddr*)&address, socketAddressLength) == -1)
    {
        std::cerr << "Error: bind failed!!!" << std::endl;
        close(socketFd);
        return 1;
    }

    // 3. Start listening
    if (listen(socketFd, MAX_PENDING_QUEUE) == -1)
    {
        std::cerr << "Error: listen failed!!!" << std::endl;
        close(socketFd);
        return 1;
    }

    std::cout << "Server waiting on port " << LISTENNING_PORT << "..." << std::endl;

    // 4. Set up poll
    std::vector<struct pollfd> fds;
    struct pollfd server_fd;
    server_fd.fd = socketFd;
    server_fd.events = POLLIN; // Watch for incoming data
    fds.push_back(server_fd);

    std::vector<int> client_sockets;

    while (true)
    {
        // Wait for an event on the sockets
        int poll_count = poll(fds.data(), fds.size(), -1);
        if (poll_count == -1)
        {
            std::cerr << "Error: poll failed!!!" << std::endl;
            break;
        }

        // Iterate through the monitored sockets
        for (size_t i = 0; i < fds.size(); ++i)
        {
            if (fds[i].revents & POLLIN)
            {
                // New connection on the listening socket
                if (fds[i].fd == socketFd)
                {
                    sockaddr_in client_address;
                    socklen_t addrlen = sizeof(client_address);
                    int client_socket = accept(socketFd, (sockaddr*)&client_address, &addrlen);
                    if (client_socket == -1)
                    {
                        std::cerr << "Error: client acceptance failed!!!" << std::endl;
                        continue;
                    }

                    // Add the client to the list
                    if (client_sockets.size() < MAX_CLIENTS)
                    {
                        std::cout << "New client connected: " << client_socket << std::endl;
                        struct pollfd client_fd;
                        client_fd.fd = client_socket;
                        client_fd.events = POLLIN;
                        fds.push_back(client_fd);
                        client_sockets.push_back(client_socket);
                    }
                    else
                    {
                        std::cerr << "Client limit reached!" << std::endl;
                        close(client_socket);
                    }
                }
                // Data received from a client
                else
                {
                    char buffer[BUFFER_SIZE] = {0};
                    int receivedBytes = recv(fds[i].fd, buffer, sizeof(buffer) - 1, 0);
                    if (receivedBytes <= 0)
                    {
                        // Client disconnected or error
                        std::cout << "Client disconnected: " << fds[i].fd << std::endl;
                        close(fds[i].fd);
                        // Remove from the list
                        for (auto it = client_sockets.begin(); it != client_sockets.end(); ++it)
                        {
                            if (*it == fds[i].fd)
                            {
                                client_sockets.erase(it);
                                break;
                            }
                        }
                        fds.erase(fds.begin() + i);
                        --i; // Adjust index after erasure
                    }
                    else
                    {
                        std::cout << "Message received from " << fds[i].fd << ": " << buffer << std::endl;

                        // Send a response
                        const char* msg = "Hello client: I am the server";
                        int sentBytes = send(fds[i].fd, msg, strlen(msg), 0);
                        if (sentBytes == -1)
                        {
                            std::cerr << "Error: sending message failed!!!" << std::endl;
                        }
                    }
                }
            }
        }
    }

    // Close all sockets
    for (int client_fd : client_sockets)
    {
        close(client_fd);
    }
    close(socketFd);

    return 0;
}