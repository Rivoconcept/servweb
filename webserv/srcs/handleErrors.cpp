/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   handleErrors.cpp                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: rhanitra <rhanitra@student.42antananari    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/09/22 19:27:14 by rhanitra          #+#    #+#             */
/*   Updated: 2025/09/22 19:28:48 by rhanitra         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/handleErrors.hpp"

std::map<int, std::string> HandleErrors::initReasonMap() {
    std::map<int, std::string> reasons;
    reasons[400] = "Bad Request";
    reasons[403] = "Forbidden";
    reasons[404] = "Not Found";
    reasons[405] = "Method Not Allowed";
    reasons[413] = "Payload Too Large";
    reasons[500] = "Internal Server Error";
    reasons[501] = "Not Implemented";
    return reasons;
}

std::string HandleErrors::getDefaultReason(int code) {
    static std::map<int, std::string> reasons = initReasonMap();
    if (reasons.find(code) != reasons.end())
        return reasons[code];
    return "Unknown Error";
}

std::string HandleErrors::getErrorBodyFromFile(
    const std::string &filePath, int code, const std::string &reason
) {
    std::ifstream file(filePath.c_str());
    if (!file.is_open()) {
        // fallback → page par défaut
        std::ostringstream oss;
        oss << "<html><head><title>" << code << " " << reason 
            << "</title></head><body><h1>" 
            << code << " " << reason 
            << "</h1></body></html>";
        return oss.str();
    }

    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string HandleErrors::generateErrorResponse(int code, const ServerConfig &serverConf,
    const LocationConfig *locationConf, const std::string &extraHeaders)
{
    std::string reason = getDefaultReason(code);

    // Chercher d’abord si la location définit une page d’erreur
    std::map<int, std::string>::const_iterator it;
    if (locationConf) {
        it = locationConf->errorPages.find(code);
        if (it != locationConf->errorPages.end()) {
            std::string filePath = locationConf->root + it->second;
            std::string body = getErrorBodyFromFile(filePath, code, reason);
            std::ostringstream oss;
            oss << "HTTP/1.1 " << code << " " << reason << "\r\n";
            oss << "Content-Type: text/html\r\n";
            if (!extraHeaders.empty())
                oss << extraHeaders;
            oss << "Content-Length: " << body.size() << "\r\n\r\n";
            oss << body;
            return oss.str();
        }
    }

    // Sinon → chercher côté server
    it = serverConf.errorPages.find(code);
    if (it != serverConf.errorPages.end()) {
        std::string filePath = serverConf.root + it->second;
        std::string body = getErrorBodyFromFile(filePath, code, reason);
        std::ostringstream oss;
        oss << "HTTP/1.1 " << code << " " << reason << "\r\n";
        oss << "Content-Type: text/html\r\n";
        if (!extraHeaders.empty())
            oss << extraHeaders;
        oss << "Content-Length: " << body.size() << "\r\n\r\n";
        oss << body;
        return oss.str();
    }

    // Sinon → fallback générique
    std::string body = getErrorBodyFromFile("", code, reason);
    std::ostringstream oss;
    oss << "HTTP/1.1 " << code << " " << reason << "\r\n";
    oss << "Content-Type: text/html\r\n";
    if (!extraHeaders.empty())
        oss << extraHeaders;
    oss << "Content-Length: " << body.size() << "\r\n\r\n";
    oss << body;
    return oss.str();
}


