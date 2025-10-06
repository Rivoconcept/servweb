#include "../include/utils.hpp"

void saveUploadedFile(const std::string &uploadDir,
                      const std::string &filename,
                      const std::string &fileContent)
{
    std::string fullPath = uploadDir + "/" + filename;
    std::ofstream out(fullPath.c_str(), std::ios::binary);
    if (out.is_open()) {
        out.write(fileContent.c_str(), fileContent.size());
        out.close();
    }
}
