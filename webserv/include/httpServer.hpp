/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:37:39 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/11 18:17:51 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "httpConfig.hpp"
#include "httpRequest.hpp"
#include "utils.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <map>
#include <string>

static const size_t BUFFER_SIZE = 1024;
static const int MAX_PENDING_QUEUE = 10;
static const int MAX_CLIENTS = 100;

class Server
{
    private:
        HttpConfig _config;
        std::vector<struct pollfd> _fds;
        std::vector<int> _clientSockets;

        void setupListeningSockets();
        void handleNewConnection(size_t index);
        void handleClientData(size_t index);

    public:
        Server(const HttpConfig &config);
        ~Server();


        void run();
};

#endif
