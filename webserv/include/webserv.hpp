/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 13:30:58 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/08 20:00:39 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include "utils.hpp"
#include <iostream>
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <map>

struct LocationConfig
{
    std::string path;                    // URL de la location, ex: "/images"
    std::string root;                    // root spécifique à la location
    std::vector<std::string> indexFiles; // fichiers par défaut
    std::vector<std::string> methods;    // GET, POST, DELETE
    bool autoindex;                      // true si autoindex on
    int returnCode;                      // code de retour HTTP (ex: 301, 403)
    std::string returnPath;              // chemin pour return ou redirection
    std::string cgiExtension;            // ex: ".php"
    std::string cgiPath;                 // chemin du binaire CGI
    std::string uploadDir;                // dossier upload
    std::string defaultFile;              // fichier par défaut si dossier
    std::map<std::string,std::string> directives; // fallback pour directives inconnues

    LocationConfig() : autoindex(false), returnCode(0) {}
};

struct ServerConfig
{
    std::string host;                     // ip ou hostname
    int listenPort;                       // port
    std::string root;                     // root du serveur
    std::vector<std::string> indexFiles;  // fichiers par défaut
    size_t clientMaxBodySize;             // max body size en octets
    std::map<int,std::string> errorPages; // code -> path
    std::vector<LocationConfig> locations;

    ServerConfig() : listenPort(0), clientMaxBodySize(0) {}
};

struct HttpConfig
{
    std::vector<ServerConfig> servers;
};

class ConfigParser
{
    private:
        std::string _filePath;
        std::string _fileContent;

        void expectChar(std::istream &input, char expected);
        void parseHttpBlock(std::istream &input, HttpConfig &httpConfig);
        void parseServerBlock(std::istream &input, ServerConfig &config);
        void parseLocationBlock(std::istream &input, LocationConfig &loc);

    public:
        ConfigParser(const std::string &filePath);
        ConfigParser(const ConfigParser& other);
        ConfigParser& operator=(const ConfigParser& other);
        ~ConfigParser();

        HttpConfig parse();

};

#endif
