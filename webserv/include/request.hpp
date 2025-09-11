

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "utils.hpp"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

struct HttpRequest
{
    // Ligne de requête
    std::string method;        // GET, POST, DELETE
    std::string uri;           // /images/logo.png
    std::string queryString;   // q=42
    std::string fragment;      // #section
    std::string httpVersion;   // HTTP/1.1

    // Headers
    std::map<std::string, std::string> headers;
    std::string host;          // extrait de "Host: ..."
    int port;

    // Corps de la requête
    std::string body;
    size_t contentLength;
    std::string boundary;      // pour multipart/form-data

    // Infos utiles après résolution
    std::string resolvedPath;  // chemin absolu sur le disque
};


#endif


