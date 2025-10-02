/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rivoinfo <rivoinfo@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/11 17:37:39 by rhanitra          #+#    #+#             */
/*   Updated: 2025/10/02 15:49:03 by rivoinfo         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPSERVER_HPP
#define HTTPSERVER_HPP

#include "httpConfig.hpp"
#include "httpRequest.hpp"
#include "handleErrors.hpp"
#include "utils.hpp"

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <set>
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
        std::map<int, ServerConfig*> _listenSockets;
        std::map<int, ServerConfig*> _clientToServer;
        MimeTypes &_mimeTypes;

        void setupListeningSockets();
        void handleNewConnection(size_t index);
        void handleClientData(size_t index);
        bool handleMultipart(const HttpRequest &req, const std::string &rawRequest,
                             int client_fd, ServerConfig *serverConf, const LocationConfig *locationConf);

    public:
        Server(const HttpConfig &config, MimeTypes &types);
        ~Server();


        void run();
};

#endif
