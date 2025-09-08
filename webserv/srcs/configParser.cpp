/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   configParser.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:10:20 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/08 19:59:55 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/webserv.hpp"

ConfigParser::ConfigParser(const std::string &filePath) : _filePath(filePath)
{
    _fileContent = ftPutFileContent(_filePath);
    
    std::vector<std::string> tab = ftSplitStr(_fileContent, "server");
    
    for (std::vector<std::string>::iterator it = tab.begin(); it != tab.end(); ++it)
    {
        std::cout << *it << std::endl;
    }
    
}

ConfigParser::ConfigParser(const ConfigParser& other)
{
    *this = other;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
    if (this != &other)
    {
        _filePath = other._filePath;
        _fileContent = other._fileContent;
    }
    return (*this);
}

ConfigParser::~ConfigParser() {}
