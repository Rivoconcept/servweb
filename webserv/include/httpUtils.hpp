#ifndef HTTPUTILS_HPP
#define HTTPUTILS_HPP

#include <string>
#include <vector>

// Normalise relativePath et résout '.' et '..'
std::string normalizeRelativePath(const std::string &relative);

// Vérifie que targetPath canonique est bien à l'intérieur de root canonique
bool isPathInsideRoot(const std::string &root, const std::string &target, std::string &outCanonicalTarget);

// Génère un autoindex HTML pour un dossier
std::string generateAutoindexHTML(const std::string &dirPath, const std::string &uri);

// Parse un header CGI et retourne une status line si Status: présent
std::string parseCGIStatusFromHeaders(const std::string &headers);

// Vérifie si contentLength dépasse clientMaxBodySize
bool checkClientMaxBodySize(size_t contentLength, size_t clientMaxBodySize);

#endif
