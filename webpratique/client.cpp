#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main() {
    // 1. Créer la socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Définir l’adresse du serveur
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // 3. Se connecter au serveur
    connect(sock, (sockaddr*)&serv_addr, sizeof(serv_addr));

    // 4. Envoyer un message
    const char* msg = "Hello";
    send(sock, msg, strlen(msg), 0);

    // 5. Recevoir la réponse
    char buffer[1024] = {0};
    read(sock, buffer, sizeof(buffer));
    std::cout << "Réponse du serveur : " << buffer << std::endl;

    // 6. Fermer
    close(sock);

    return 0;
}
