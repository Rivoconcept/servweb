/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:25:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/10/06 17:09:15 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpServer.hpp"
#include "../include/httpResponse.hpp"


Server::Server(const HttpConfig &config, MimeTypes &types) : _config(config), _mimeTypes(types)
{
    setupListeningSockets();
}

Server::~Server()
{
    for (size_t i = 0; i < _clientSockets.size(); ++i)
        close(_clientSockets[i]);
    for (size_t i = 0; i < _fds.size(); ++i)
        close(_fds[i].fd);
}

void Server::setupListeningSockets()
{
    for (size_t i = 0; i < _config.servers.size(); ++i)
    {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock == -1)
        {
            std::cerr << "Error: socket init failed\n";
            continue;
        }

        int opt = 1;
        setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY; // TODO: convertir host string en in_addr
        addr.sin_port = htons(_config.servers[i].listenPort);

        if (bind(sock, (sockaddr*)&addr, sizeof(addr)) == -1)
        {
            std::cerr << "Error: bind failed on port " << _config.servers[i].listenPort << "\n";
            close(sock);
            continue;
        }

        if (listen(sock, MAX_PENDING_QUEUE) == -1)
        {
            std::cerr << "Error: listen failed on port " << _config.servers[i].listenPort << "\n";
            close(sock);
            continue;
        }

        struct pollfd pfd;
        pfd.fd = sock;
        pfd.events = POLLIN;
        _fds.push_back(pfd);

        // 👇 Ici : association socket d'écoute ↔ config serveur
        _listenSockets[sock] = &_config.servers[i];

        std::cout << "Listening on " << _config.servers[i].host 
                << ":" << _config.servers[i].listenPort << std::endl;
    }
}

void Server::handleNewConnection(size_t index)
{
    sockaddr_in client_addr;
    socklen_t addrlen = sizeof(client_addr);
    int client_sock = accept(_fds[index].fd, (sockaddr*)&client_addr, &addrlen);
    if (client_sock != -1)
    {
        struct pollfd client_pfd;
        client_pfd.fd = client_sock;
        client_pfd.events = POLLIN;
        _fds.push_back(client_pfd);
        _clientSockets.push_back(client_sock);

        // Vérifie que le fd d’écoute est bien dans la map
        if (_listenSockets.count(_fds[index].fd)) {
            _clientToServer[client_sock] = _listenSockets[_fds[index].fd];
            std::cout << "New client " << client_sock 
                      << " attached to server listening on port "
                      << _listenSockets[_fds[index].fd]->listenPort << std::endl;
        } else {
            std::cerr << "Error: listening socket " << _fds[index].fd 
                      << " not found in _listenSockets!" << std::endl;
        }
    }
}

/*bool Server::handleMultipart(const HttpRequest &req, const std::string &rawRequest,
                             int client_fd, ServerConfig *serverConf, const LocationConfig *locationConf)
{
    std::string contentType;
    if (req.headers.find("Content-Type") != req.headers.end())
        contentType = req.headers.at("Content-Type");

    if (contentType.find("multipart/form-data") == std::string::npos)
        return false; // ❌ Pas un upload multipart → on laisse le handler normal continuer

    // ✅ 1) Boundary
    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos) {
        std::string response = HandleErrors::generateErrorResponse(
            400, *serverConf, locationConf
        );
        send(client_fd, response.c_str(), response.size(), 0);
        return true; // ✅ On a traité (erreur)
    }
    std::string boundary = contentType.substr(pos + 9);

    // ✅ 2) Body brut
    size_t bodyPos = rawRequest.find("\r\n\r\n");
    if (bodyPos == std::string::npos) {
        std::string response = HandleErrors::generateErrorResponse(
            400, *serverConf, locationConf
        );
        send(client_fd, response.c_str(), response.size(), 0);
        return true;
    }
    std::string body = rawRequest.substr(bodyPos + 4);
    std::string delimiter = "--" + boundary;

    // ✅ 3) Split en parties
    std::vector<std::string> parts = splitParts(body, delimiter);

    // ✅ 4) Dossier d’upload
    std::string uploadDir;
    if (locationConf && !locationConf->root.empty())
        uploadDir = locationConf->root;
    else
        uploadDir = serverConf->root;

    // ✅ 5) Parcourir chaque part
    for (size_t i = 0; i < parts.size(); ++i)
    {
        std::string filename = extractFilename(parts[i]);  
        std::string fieldData = extractFileContent(parts[i]); 

        if (!filename.empty()) {
            std::string fullpath = uploadDir + "/" + filename;
            std::ofstream file(fullpath.c_str(), std::ios::binary);
            if (!file.is_open()) {
                std::cerr << "Erreur : impossible d’écrire dans " << fullpath << std::endl;
            } else {
                file.write(fieldData.c_str(), fieldData.size());
                file.close();
            }
        }

    }

    // ✅ 6) Réponse HTTP
    std::string response =
        "HTTP/1.1 201 Created\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";

    send(client_fd, response.c_str(), response.size(), 0);
    return true; // ✅ Upload traité ici
}*/


void Server::handleMultipartUpload(const HttpRequest &req,
                           const std::string &rawRequest,
                           const std::string &uploadDir,
                           ServerConfig *serverConf,
                           const LocationConfig *locationConf,
                           int client_fd)
{
    // ✅ 1) Récupérer boundary dans Content-Type
    std::string contentType = req.headers.find("Content-Type") != req.headers.end()
                              ? req.headers.find("Content-Type")->second
                              : "";

    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
    {
        std::string response = HandleErrors::generateErrorResponse(
            400, *serverConf, locationConf
        );
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }

    std::string boundary = contentType.substr(pos + 9);
    std::string delimiter = "--" + boundary;

    // ✅ 2) Séparer le body
    std::string body = rawRequest.substr(rawRequest.find("\r\n\r\n") + 4);
    std::vector<std::string> parts = splitParts(body, delimiter);

    // ✅ 3) Map des champs textes
    std::map<std::string, std::string> fields;
    fields["FirstName"] = "";
    fields["Name"] = "";
    fields["sex"] = "";
    fields["BirthDay"] = "";
    fields["status"] = "";
    fields["phone"] = "";
    fields["email"] = "";
    fields["information"] = "";

    std::string uploadedFileName = "";

    for (size_t i = 0; i < parts.size(); ++i) {
        std::string disposition;
        std::string content;
        size_t headerEnd = parts[i].find("\r\n\r\n");
        if (headerEnd == std::string::npos)
            continue;

        std::string headersPart = parts[i].substr(0, headerEnd);
        content = parts[i].substr(headerEnd + 4);

        if (content.size() >= 2 && content.substr(content.size() - 2) == "\r\n") {
            content.erase(content.size() - 2);
        }

        std::istringstream iss(headersPart);
        std::string line;
        while (std::getline(iss, line)) {
            if (line.find("Content-Disposition:") != std::string::npos) {
                disposition = line;
                break;
            }
        }

        size_t namePos = disposition.find("name=\"");
        if (namePos == std::string::npos)
            continue;
        namePos += 6;
        size_t nameEnd = disposition.find("\"", namePos);
        std::string fieldName = disposition.substr(namePos, nameEnd - namePos);

        size_t filePos = disposition.find("filename=\"");
        if (filePos != std::string::npos) {
            filePos += 10;
            size_t fileEnd = disposition.find("\"", filePos);
            uploadedFileName = disposition.substr(filePos, fileEnd - filePos);

            if (!uploadedFileName.empty())
                saveUploadedFile(uploadDir, uploadedFileName, content);
        } else if (fields.find(fieldName) != fields.end()) {
            fields[fieldName] = content;
        }
    }

    // ✅ Sauvegarder CSV
    std::string csvPath = uploadDir + "/contacts.csv";
    appendToCSV(fields, csvPath, uploadedFileName);

    // ✅ 201 Created
    std::string response =
        "HTTP/1.1 201 Created\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";

    send(client_fd, response.c_str(), response.size(), 0);
}

void Server::saveUploadedFile(const std::string &uploadDir,
                      const std::string &filename,
                      const std::string &fileContent)
{
    std::string fullPath = uploadDir + "/" + filename;
    std::ofstream out(fullPath.c_str(), std::ios::binary);
    if (out.is_open()) {
        out.write(fileContent.c_str(), fileContent.size());
        out.close();
    }
}

void Server::handleClientData(size_t index)
{
    char buffer[BUFFER_SIZE] = {0};
    int client_fd = _fds[index].fd;

    int received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        close(client_fd);
        _clientToServer.erase(client_fd);
        _fds.erase(_fds.begin() + index);
        return;
    }

    std::string rawRequest(buffer, received);
    HttpRequestParser parser;
    HttpRequest req;

    try {
        req = parser.parseRequest(rawRequest);
    } 
    catch (std::exception &e)
    {
        // 🔹 Requête malformée → 400 Bad Request
        std::string response = HandleErrors::generateErrorResponse(
            400, *_clientToServer[client_fd], NULL
        );
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }
    // 🔹 Récupérer la config du serveur associé
    ServerConfig *serverConf = _clientToServer[client_fd];
    if (!serverConf) {
        std::cerr << "Error: no server config found for client " << client_fd << "\n";
        return;
    }

    // 🔹 Trouver la meilleure location
    const LocationConfig* locationConf = NULL;
    size_t bestMatchLen = 0;
    for (size_t i = 0; i < serverConf->locations.size(); ++i) {
        const LocationConfig &loc = serverConf->locations[i];
        if (req.uri.find(loc.path) == 0 && loc.path.size() > bestMatchLen) {
            locationConf = &loc;
            bestMatchLen = loc.path.size();
        }
    }

    // fallback si aucune location spécifique trouvée
    if (!locationConf && !serverConf->locations.empty())
        locationConf = &serverConf->locations[0];

    // 🔹 Vérifier méthode autorisée (avec fallback par défaut)
    std::set<std::string> allowed;
    if (locationConf && !locationConf->methods.empty()) {
        allowed.insert(locationConf->methods.begin(), locationConf->methods.end());
    } else {
        allowed.insert("GET");
        allowed.insert("POST");
        allowed.insert("DELETE");
    }

    if (allowed.find(req.method) == allowed.end()) {
        std::string response = HandleErrors::generateErrorResponse(
            405, *serverConf, locationConf, "Allow: GET, POST, DELETE\r\n"
        );
        send(client_fd, response.c_str(), response.size(), 0);
        return;
    }

    if (req.method == "POST") {
        std::string contentType = req.headers["Content-Type"];
        if (contentType.find("multipart/form-data") != std::string::npos) {

            // ✅ Déterminer le répertoire d'upload
            std::string uploadDir;
            if (locationConf && !locationConf->root.empty())
                uploadDir = locationConf->root;
            else
                uploadDir = serverConf->root;  // fallback

            // ✅ Appeler le handler multipart
            handleMultipartUpload(req, rawRequest, uploadDir, serverConf, locationConf, client_fd);
            return;
        }
    }



    // 🔹 Construire et envoyer la réponse HTTP
    HttpResponseBuilder builder(_mimeTypes);
    std::string response;
    try {
        response = builder.buildResponse(req, *serverConf, 
                                         locationConf ? *locationConf : LocationConfig());
    } catch (std::exception &e) {
        response = HandleErrors::generateErrorResponse(500, *serverConf, locationConf);
    }

    send(client_fd, response.c_str(), response.size(), 0);
}


void Server::run()
{
    while (true)
    {
        int poll_count = poll(_fds.data(), _fds.size(), -1);
        if (poll_count == -1)
        {
            std::cerr << "Error: poll failed\n";
            break;
        }

        for (size_t i = 0; i < _fds.size(); ++i)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_listenSockets.count(_fds[i].fd)) {
                    // C’est un socket d’écoute
                    handleNewConnection(i);
                }
                else {
                    // C’est un client
                    handleClientData(i);
                }
            }
        }

    }
}

