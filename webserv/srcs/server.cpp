/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:25:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/10/07 15:16:48 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpServer.hpp"
#include "../include/httpResponse.hpp"
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include "../include/httpUtils.hpp"


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



void Server::handleMultipartUpload(const HttpRequest &req, const std::string &rawRequest, const std::string &uploadDir, int client_fd)

{
    std::string contentType;
    if (req.headers.find("Content-Type") != req.headers.end())
        contentType = req.headers.find("Content-Type")->second;

    if (contentType.find("multipart/form-data") == std::string::npos)
        return;

    size_t pos = contentType.find("boundary=");
    if (pos == std::string::npos)
        return;

    std::string boundary = contentType.substr(pos + 9);
    std::string body = rawRequest.substr(rawRequest.find("\r\n\r\n") + 4);
    std::string delimiter = "--" + boundary;

    std::vector<std::string> parts = splitParts(body, delimiter);

    std::map<std::string, std::string> fields;
    std::string uploadedFilename;

    for (size_t i = 0; i < parts.size(); ++i)
    {
        std::string name = extractFieldName(parts[i]);
        std::string filename = extractFilename(parts[i]);
        std::string data = extractFileContent(parts[i]);

        if (!filename.empty()) {
            saveUploadedFile(uploadDir, filename, data);
            uploadedFilename = filename;
        } else if (!name.empty()) {
            fields[name] = data;
        }
    }

    // Ajout du fichier CSV
    appendToCSV(fields, uploadDir + "/contacts.csv", uploadedFilename);

    std::string response =
        "HTTP/1.1 201 Created\r\n"
        "Content-Length: 0\r\n"
        "Connection: close\r\n\r\n";

    HandleErrors::sendResponse(client_fd, response);
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

    std::string rawRequest;
    // Read from socket until we have received headers and the full body
    while (true) {
        int received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (received < 0) {
            // recv error: close client
            close(client_fd);
            _clientToServer.erase(client_fd);
            _fds.erase(_fds.begin() + index);
            return;
        }
        if (received == 0) {
            // connection closed by peer
            break;
        }
        rawRequest.append(buffer, received);

        // Do we have the end of headers?
        size_t hdrEnd = rawRequest.find("\r\n\r\n");
        if (hdrEnd != std::string::npos) {
            // Try to find Content-Length header
            size_t contentLenPos = rawRequest.find("Content-Length:");
            if (contentLenPos != std::string::npos) {
                // extract number after header
                size_t lineEnd = rawRequest.find("\r\n", contentLenPos);
                if (lineEnd == std::string::npos)
                    lineEnd = hdrEnd; // defensive
                size_t valStart = contentLenPos + strlen("Content-Length:");
                std::string val = rawRequest.substr(valStart, lineEnd - valStart);
                // trim spaces
                size_t first = val.find_first_not_of(" \t");
                size_t last = val.find_last_not_of(" \t");
                if (first != std::string::npos && last != std::string::npos)
                    val = val.substr(first, last - first + 1);
                else
                    val = "0";

                size_t contentLength = static_cast<size_t>(atoi(val.c_str()));
                size_t bodyLen = rawRequest.size() - (hdrEnd + 4);
                if (bodyLen >= contentLength) {
                    // we have whole body
                    break;
                }
                // else continue reading
            } else {
                // No Content-Length => can't determine body length; break and let parser handle it
                break;
            }
        }
        // else continue reading
        // small safeguard: if rawRequest grows too large without headers, stop to avoid DoS
        if (rawRequest.size() > 10 * 1024 * 1024) // 10MB
            break;
    }
    HttpRequestParser parser;
    HttpRequest req;

    try {
        req = parser.parseRequest(rawRequest);
    } 
    catch (std::exception &e)
    {
        // 🔹 Requête malformée → 400 Bad Request
        if (_clientToServer.count(client_fd) && _clientToServer[client_fd])
            HandleErrors::sendError(client_fd, 400, *_clientToServer[client_fd], NULL);
        else
            std::cerr << "handleClientData: client config missing when sending 400\n";
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

    // Enforcer client_max_body_size (après détermination de la location)
    if (checkClientMaxBodySize(req.contentLength, serverConf->clientMaxBodySize)) {
        HandleErrors::sendError(client_fd, 413, *serverConf, locationConf);
        return;
    }

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
        HandleErrors::sendError(client_fd, 405, *serverConf, locationConf, "Allow: GET, POST, DELETE\r\n");
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
            handleMultipartUpload(req, rawRequest, uploadDir, client_fd);
            return;
        }
    }

    // DELETE handler
    if (req.method == "DELETE") {
        // Calculer le répertoire root
        std::string root = (locationConf && !locationConf->root.empty()) ? locationConf->root : serverConf->root;

        // Calculer path brut relatif à la location
        std::string rawRel;
        if (locationConf && !locationConf->path.empty() && req.uri.find(locationConf->path) == 0)
            rawRel = req.uri.substr(locationConf->path.size());
        else if (!req.uri.empty() && req.uri[0] == '/')
            rawRel = req.uri.substr(1);
        else
            rawRel = req.uri;

        if (rawRel.empty()) {
            HandleErrors::sendError(client_fd, 403, *serverConf, locationConf);
            return;
        }

        std::string normRel = normalizeRelativePath(rawRel);
        std::string targetPath = root + normRel;

        std::string canonTarget;
        if (!isPathInsideRoot(root, targetPath, canonTarget)) {
            HandleErrors::sendError(client_fd, 403, *serverConf, locationConf);
            return;
        }

        struct stat st;
        if (stat(canonTarget.c_str(), &st) == -1) {
            HandleErrors::sendError(client_fd, 404, *serverConf, locationConf);
            return;
        }
        if (S_ISDIR(st.st_mode)) {
            // refuse delete of directory for now
            HandleErrors::sendError(client_fd, 403, *serverConf, locationConf);
            return;
        }

        if (unlink(canonTarget.c_str()) == -1) {
            HandleErrors::sendError(client_fd, 500, *serverConf, locationConf);
            return;
        }

        std::string resp = "HTTP/1.1 204 No Content\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
        HandleErrors::sendResponse(client_fd, resp);
        return;
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

    HandleErrors::sendResponse(client_fd, response);
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

