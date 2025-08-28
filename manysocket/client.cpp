#include "clientweb.hpp"

int main() {
    // 1. Créer la socket
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd == -1)
    {
         std::cerr << "Error: socket initialisation failed!!!" << std::endl;
         exit(1);
    }

    // 2. Définir l’adresse du serveur
    sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    socketAddress.sin_port = htons(LISTENNING_PORT);
    int inetReturnCode = inet_pton(AF_INET, "127.0.0.1", &socketAddress.sin_addr);
    if (inetReturnCode == -1)
    {
         std::cerr << "Error: invalid or unsupported address!!!" << std::endl;
         exit(1);
    }

    // 3. Se connecter au serveur
    int socketAddressLength = sizeof(socketAddress);
    int connectionStatus = connect(socketFd, (sockaddr*)&socketAddress, socketAddressLength);
    if (connectionStatus == -1)
    {
         std::cerr << "(CLIENT): Server connection failed to server!!!" << std::endl;
         exit(1);
    }

    // 4. Envoyer un message
    const char* msg = "Bonjour serveur, je suis le client";
    int sentBytes = send(socketFd, msg, strlen(msg), 0);
    if (sentBytes == -1)
    {
        std::cerr << "(CLIENT): Sending message failed to server!!!" << std::endl;
        exit(1);         
    }

    // 5. Recevoir la réponse
    char buffer[BUFFER_SIZE] = {0};
    int receivedBytes = recv(socketFd, buffer, sizeof(buffer), 0);
    if (receivedBytes == -1)
    {
        std::cerr << "(CLIENT): Message reception failed!!!" << std::endl;
        exit(1);        
    }


     std::cout << "Serveur: " << buffer << std::endl;
    // 6. Fermer
    close(socketFd);

    return 0;
}
