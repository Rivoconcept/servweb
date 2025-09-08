/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   putFileContent.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/08 19:31:19 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/08 19:39:13 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/utils.hpp"

std::string ftPutFileContent(const std::string& fileName)
{
    std::string fileContent;
    std::ifstream ifs(fileName.c_str());
    if (!ifs)
        throw std::runtime_error("could not open " + fileName);

    std::string line;
    while (std::getline(ifs, line))
    {
        fileContent += line;
        fileContent += "\n";
    }
    ifs.close();
    return (fileContent);
}