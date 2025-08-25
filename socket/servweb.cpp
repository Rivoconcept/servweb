#include "webserv.hpp"

int main()
{
    // 1. Créer la socket
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd == -1)
    {
         std::cerr << "Error: socket initialisation failed!!!" << std::endl;
         exit(1);
    }

    // 2. Associer l’IP et le port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // accepte toutes les IP
    address.sin_port = htons(LISTENNING_PORT);        // port 8080

    int socketAddressLength = sizeof(address);
    int bindReturncode = bind(socketFd, (sockaddr*)&address, socketAddressLength);
    if (bindReturncode == -1)
    {
        std::cerr << "socket connexion failed!!!" << std::endl;
        exit(1);
    }

    // 3. Mettre en écoute
    if (listen(socketFd, MAX_PENDING_QUEUE) == -1)
    {
        std::cerr << "Failed to start listenning!!!" << std::endl;
        exit(1);
    }

    std::cout << "Serveur en attente..." << std::endl;

    // 4. Accepter un client
    int addrlen = sizeof(address);
    int client_socket = accept(socketFd, (sockaddr*)&address, (socklen_t*)&addrlen);

    if (client_socket == -1)
    {
        std::cerr << "Connection establishment failed!!!" << std::endl;
        exit(1);
    }
    // 5. Recevoir un message
    char buffer[BUFFER_SIZE] = {0};
    int receivedBytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (receivedBytes == -1)
    {
        std::cerr << "Message reception failed!!!" << std::endl;
        exit(1);        
    }
    std::cout << "Message reçu : " << buffer << std::endl;

    // 6. Envoyer une réponse
    const char* msg = "Bonjour client: je suis le serveur";
    int sentBytes = send(client_socket, msg, strlen(msg), 0);
    if (sentBytes == -1)
    {
        std::cerr << "Sending message failed!!!" << std::endl;
        exit(1);         
    }

    // 7. Fermer
    close(client_socket);
    close(socketFd);

    return 0;
}
