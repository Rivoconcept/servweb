#include "../include/utils.hpp"
#include <iostream>

// Helper utilities at file scope (compatible with C++98)
static std::string sanitizeString(const std::string &s)
{
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\n' || c == '\r' || c == ';')
            out += ' ';
        else
            out += c;
    }
    return out;
}

static std::string getFieldValue(const std::map<std::string, std::string> &fieldsMap, const std::string &key)
{
    std::map<std::string, std::string>::const_iterator it = fieldsMap.find(key);
    if (it != fieldsMap.end())
        return sanitizeString(it->second);
    return std::string();
}

void appendToCSV(const std::map<std::string, std::string> &fields,
                 const std::string &csvPath,
                 const std::string &filename)
{
    std::ofstream csv(csvPath.c_str(), std::ios::app);
    if (!csv.is_open()) {
        std::cerr << "appendToCSV: impossible d'ouvrir le fichier " << csvPath << "\n";
        return;
    }

    // Si le fichier est vide, ajouter l’en-tête
    std::ifstream check(csvPath.c_str());
    bool empty = check.peek() == std::ifstream::traits_type::eof();
    check.close();

    if (empty) {
        csv << "FirstName;Name;sex;BirthDay;status;phone;email;information;filename\n";
    }

    // ... helper functions are defined at file scope above

    csv
        << getFieldValue(fields, "FirstName") << ";"
        << getFieldValue(fields, "Name") << ";"
        << getFieldValue(fields, "sex") << ";"
        << getFieldValue(fields, "BirthDay") << ";"
        << getFieldValue(fields, "status") << ";"
        << getFieldValue(fields, "phone") << ";"
        << getFieldValue(fields, "email") << ";"
        << getFieldValue(fields, "information") << ";"
        << sanitizeString(filename) << "\n";

    csv.flush();
    csv.close();
}
