/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/28 14:43:48 by rhanitra          #+#    #+#             */
/*   Updated: 2025/08/28 14:49:56 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "./include/webserv.hpp"




int main(int argc, char **argv)
{
    if (argc != 2 || (argc == 2 && !argv[1][0]))
    {
        std::cerr << "Use parameter: ./webserv <file config path>" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    std::istringstream iss(argv[1]);
    std::string file;
    
    if (!(iss >> file))
        throw std::runtime_error("Error: could not open file.");

    try
    {
        ConfigParser parser(file);
        HttpConfig config = parser.parse();

        std::cout << "Parsed " << config.servers.size() << " server block(s)." << std::endl;
        for (size_t i = 0; i < config.servers.size(); ++i)
        {
            std::cout << "Server " << i << " listens on port " << config.servers[i].listenPort << std::endl;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Config error: " << e.what() << std::endl;
    }
}