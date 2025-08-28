/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:10:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/08/28 15:03:14 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

ConfigParser::ConfigParser(const std::string &filePath) : _filePath(filePath) {}

ConfigParser::ConfigParser(const ConfigParser& other)
{
    *this = other;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
    if (this != &other)
    {
        _filePath = other._filePath;
    }
    return (*this);
}

ConfigParser::~ConfigParser() {}


HttpConfig ConfigParser::parse()
{
    std::ifstream file(_filePath.c_str());
    if (!file.is_open())
        throw std::runtime_error("Cannot open config file: " + _filePath);

    HttpConfig httpConfig;
    std::string token;

    while (file >> token)
    {
        if (token == "http")
        {
            expectChar(file, '{');
            parseHttpBlock(file, httpConfig);
        }
        else
        {
            throw std::runtime_error("Unexpected token at root level: " + token);
        }
    }
    return httpConfig;
}

void ConfigParser::expectChar(std::istream &input, char expected)
{
    char c;
    input >> c;
    if (c != expected)
    {
        std::stringstream ss;
        ss << "Expected '" << expected << "', got '" << c << "'";
        throw std::runtime_error(ss.str());
    }
}

int ConfigParser::toInt(const std::string &s)
{
    std::stringstream ss(s);
    int val;
    if (!(ss >> val))
        throw std::runtime_error("Invalid number: " + s);
    return val;
}

void ConfigParser::parseHttpBlock(std::istream &input, HttpConfig &httpConfig)
{
    std::string token;
    while (input >> token)
    {
        if (token == "}") break; // fin http

        if (token == "server")
        {
            ServerConfig server;
            expectChar(input, '{');
            parseServerBlock(input, server);
            httpConfig.servers.push_back(server);
        }
        else
        {
            throw std::runtime_error("Unknown directive inside http block: " + token);
        }
    }
}

void ConfigParser::parseServerBlock(std::istream &input, ServerConfig &config)
{
    std::string token;
    while (input >> token)
    {
        if (token == "}") break;

        if (token == "listen")
        {
            std::string portStr;
            if (!(input >> portStr))
                throw std::runtime_error("Missing listen port");
            config.listenPort = toInt(portStr);
            expectChar(input, ';');
        }
        else if (token == "root")
        {
            input >> config.root;
            expectChar(input, ';');
        }
        else if (token == "index")
        {
            std::string fileName;
            while (input >> fileName && fileName[fileName.size() - 1] != ';')
            {
                config.indexFiles.push_back(fileName);
            }
            if (fileName[fileName.size() - 1] == ';')
            {
                fileName = fileName.substr(0, fileName.size() - 1);
                config.indexFiles.push_back(fileName);
            }
            else
            {
                throw std::runtime_error("Missing ';' after index directive");
            }
        }
        else if (token == "location")
        {
            LocationConfig loc;
            input >> loc.path;
            expectChar(input, '{');
            parseLocationBlock(input, loc);
            config.locations.push_back(loc);
        }
        else
        {
            throw std::runtime_error("Unknown directive in server block: " + token);
        }
    }
}

void ConfigParser::parseLocationBlock(std::istream &input, LocationConfig &loc)
{
    std::string token;
    while (input >> token)
    {
        if (token == "}") break;

        std::string value;
        std::getline(input, value, ';');
        if (value.empty())
            throw std::runtime_error("Missing value for directive: " + token);

        size_t start = value.find_first_not_of(" \t");
        size_t end   = value.find_last_not_of(" \t");
        if (start != std::string::npos)
            value = value.substr(start, end - start + 1);

        loc.directives[token] = value;
    }
}
