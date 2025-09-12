#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "httpConfig.hpp"
#include "httpRequest.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

class HttpResponseBuilder
{
    private:
        const ServerConfig &_serverConf;

        std::string readFile(const std::string &path);

    public:
        HttpResponseBuilder(const ServerConfig &conf);

        std::string buildResponse(const HttpRequest &req);
};

#endif
