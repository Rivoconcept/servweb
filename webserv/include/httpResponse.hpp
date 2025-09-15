/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   httpResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/12 17:17:36 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/15 19:33:16 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "httpConfig.hpp"
#include "httpRequest.hpp"
#include "handleCGI.hpp"
#include "utils.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

class HttpResponseBuilder
{
    private:
        const ServerConfig &_serverConf;
        const MimeTypes &_mimeTypes;

    public:
        HttpResponseBuilder(const ServerConfig &conf, const MimeTypes &types);
        ~HttpResponseBuilder();


        std::string getMimeType(const std::string &path);
        std::string buildResponse(const HttpRequest &req, const ServerConfig &serverConf, const LocationConfig &locationConf);
};

#endif
