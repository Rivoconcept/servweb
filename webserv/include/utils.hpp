/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/29 16:22:27 by rhanitra          #+#    #+#             */
/*   Updated: 2025/10/03 13:30:09 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <map>


template <typename T>
std::string ftToString(T value)
{
    std::ostringstream ss;
    ss << value;
    return (ss.str());
}

int ftToInt(const std::string &s);
std::string ftStrdup(const char* s);
std::string ftReadFile(const std::string &path);
std::vector<std::string> ftSplitStr(const std::string& str, const std::string& delimiter);
std::vector<std::string> splitParts(const std::string &body, const std::string &delimiter);
std::string extractFilename(const std::string &part);
std::string extractFileContent(const std::string &part);



#endif
