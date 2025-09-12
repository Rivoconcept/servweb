/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   responseBuilder.cpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:28 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/12 19:02:03 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/httpResponse.hpp"

HttpResponseBuilder::HttpResponseBuilder(const ServerConfig &conf) : _serverConf(conf) {}

HttpResponseBuilder::~HttpResponseBuilder() {}

std::string HttpResponseBuilder::buildResponse(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &locationConf)
{
    std::string body;
    std::string statusLine = "HTTP/1.1 200 OK\r\n";
    std::string headers;

    try
    {
        if (req.uri.find(".php") != std::string::npos)
        {
            HandleCGI cgi(req, serverConf, locationConf);
            cgi.buildEnv();
            body = cgi.execute();   // à implémenter (fork/exec)
        }
        else
        {
            // fichier statique
            std::string filePath = _serverConf.root + req.uri;

            if (req.uri == "/") // si la requête est juste "/"
            {
                if (!_serverConf.indexFiles.empty())
                    filePath = _serverConf.root + "/" + _serverConf.indexFiles[0];
                else
                    filePath = _serverConf.root + "/index.html";
            }

            body = ftReadFile(filePath);
        }

        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
        headers += "Content-Type: text/html\r\n";
    }
    catch (...)
    {
        statusLine = "HTTP/1.1 404 Not Found\r\n";
        body = "<h1>404 Not Found</h1>";
        headers = "Content-Length: " + ftToString(body.size()) + "\r\n";
        headers += "Content-Type: text/html\r\n";
    }

    return statusLine + headers + "\r\n" + body;
}

