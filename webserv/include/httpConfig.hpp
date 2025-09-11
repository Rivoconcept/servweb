/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpConfig.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 13:30:58 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/11 18:08:47 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPCONFIG_HPP
#define HTTPCONFIG_HPP

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
    std::string path;
    std::string root;
    std::vector<std::string> indexFiles;
    std::vector<std::string> methods;
    bool autoindex;
    int returnCode;
    std::string returnPath;
    std::string cgiExtension;
    std::string cgiPath;
    std::string uploadDir;
    std::string defaultFile;
    std::map<std::string,std::string> directives;

    LocationConfig() : autoindex(false), returnCode(0) {}
};

struct ServerConfig
{
    std::string host;
    int listenPort;
    std::string root;
    std::vector<std::string> indexFiles;
    size_t clientMaxBodySize;
    std::map<int,std::string> errorPages;
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

        void expectToken(std::istream &input, const std::string &expected);
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
