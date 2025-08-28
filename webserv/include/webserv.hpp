/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 13:30:58 by rhanitra          #+#    #+#             */
/*   Updated: 2025/08/28 14:49:49 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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
    std::map<std::string, std::string> directives;
};

struct ServerConfig
{
    int listenPort;
    std::string root;
    std::vector<std::string> indexFiles;
    std::vector<LocationConfig> locations;

    ServerConfig() : listenPort(80) {}
};

struct HttpConfig
{
    std::vector<ServerConfig> servers;
};

class ConfigParser
{
    private:
        std::string _filePath;

        void expectChar(std::istream &input, char expected);
        int toInt(const std::string &s);
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
