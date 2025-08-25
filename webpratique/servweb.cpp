#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>

int main()
{
    // 1. Créer la socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Associer l’IP et le port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;  // accepte toutes les IP
    address.sin_port = htons(8080);        // port 8080

    bind(server_fd, (sockaddr*)&address, sizeof(address));

    // 3. Mettre en écoute
    listen(server_fd, 1);

    std::cout << "Serveur en attente..." << std::endl;

    // 4. Accepter un client
    int addrlen = sizeof(address);
    int client_socket = accept(server_fd, (sockaddr*)&address, (socklen_t*)&addrlen);

    // 5. Recevoir un message
    char buffer[1024] = {0};
    read(client_socket, buffer, sizeof(buffer));
    std::cout << "Message reçu : " << buffer << std::endl;

    // 6. Envoyer une réponse
    const char* msg = "Bonjour";
    send(client_socket, msg, strlen(msg), 0);

    // 7. Fermer
    close(client_socket);
    close(server_fd);

    return 0;
}
