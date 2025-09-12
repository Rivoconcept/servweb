#include "../include/httpResponse.hpp"

// Constructeur
HttpResponseBuilder::HttpResponseBuilder(const ServerConfig &conf) 
    : _serverConf(conf) {}

// Lire un fichier (html, etc.)
std::string HttpResponseBuilder::readFile(const std::string &path)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open file: " + path);
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Construire la réponse HTTP
std::string HttpResponseBuilder::buildResponse(const HttpRequest &req)
{
    std::string body;
    try {
        std::string filePath = _serverConf.root + req.uri;
        body = readFile(filePath);
    } catch (...) {
        body = "<h1>404 Not Found</h1>";
    }

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Content-Type: text/html\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}
