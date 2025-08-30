/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:10:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/08/30 15:16:22 by rhanitra         ###   ########.fr       */
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

void ConfigParser::parseHttpBlock(std::istream &input, HttpConfig &httpConfig)
{
    std::string token;
    while (input >> token)
    {
        if (token == "}") break;

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

        else if (token == "listen")
        {
            std::string addr;
            if (!(input >> addr))
                throw std::runtime_error("Missing listen address");

            size_t colon = addr.find(':');
            if (colon != std::string::npos)
            {
                config.host = addr.substr(0, colon);
                config.listenPort = toInt(addr.substr(colon + 1));
            }
            else
            {
                config.host = "0.0.0.0";
                config.listenPort = toInt(addr);
            }
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
        else if (token == "client_max_body_size")
        {
            std::string sizeStr;
            if (!(input >> sizeStr))
                throw std::runtime_error("Missing client_max_body_size value");

            char unit = sizeStr[sizeStr.size() - 1];
            size_t multiplier = 1;

            if (unit == 'K' || unit == 'k') multiplier = 1024, sizeStr = sizeStr.substr(0, sizeStr.size() - 1);
            else if (unit == 'M' || unit == 'm') multiplier = 1024 * 1024, sizeStr = sizeStr.substr(0, sizeStr.size() - 1);
            else if (unit == 'G' || unit == 'g') multiplier = 1024 * 1024 * 1024, sizeStr = sizeStr.substr(0, sizeStr.size() - 1);

            config.clientMaxBodySize = static_cast<size_t>(toInt(sizeStr)) * multiplier;
            expectChar(input, ';');
        }
        else if (token == "error_page")
        {
            std::string codeStr, path;
            if (!(input >> codeStr >> path))
                throw std::runtime_error("Invalid error_page directive");

            int code = toInt(codeStr);
            if (path[path.size() - 1] == ';')
                path = path.substr(0, path.size() - 1);
            else expectChar(input, ';');

            config.errorPages[code] = path;
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

        // trim
        size_t start = value.find_first_not_of(" \t");
        size_t end   = value.find_last_not_of(" \t");
        if (start != std::string::npos)
            value = value.substr(start, end - start + 1);

        // directives known to the location
        if (token == "methods")
        {
            std::stringstream ss(value);
            std::string method;
            while (ss >> method)
                loc.methods.push_back(method);
        }
        else if (token == "autoindex")
        {
            if (value == "on") loc.autoindex = true;
            else loc.autoindex = false;
        }
        else if (token == "index")
        {
            std::stringstream ss(value);
            std::string file;
            while (ss >> file)
                loc.indexFiles.push_back(file);
        }
        else if (token == "root")
            loc.root = value;
        else if (token == "upload_dir")
            loc.uploadDir = value;
        else if (token == "return")
        {
            std::stringstream ss(value);
            ss >> loc.returnCode;
            if (ss >> loc.returnPath) {} // optional path
        }
        else if (token == "redirect")
        {
            loc.returnCode = 301;   // code HTTP pour redirection permanente
            loc.returnPath = value;  // chemin de redirection
        }
        else if (token == "default_file") loc.defaultFile = value;
        else if (token == "cgi_extension") loc.cgiExtension = value;
        else if (token == "cgi_path") loc.cgiPath = value;
        else
        {
            // fallback for any unknown directive
            loc.directives[token] = value;
        }
    }
}

