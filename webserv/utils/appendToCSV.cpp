#include "../include/utils.hpp"

void appendToCSV(const std::map<std::string, std::string> &fields, 
                 const std::string &csvPath,
                 const std::string &uploadedFileName)
{
    bool fileExists = false;
    {
        std::ifstream test(csvPath.c_str());
        fileExists = test.good();
    }

    std::ofstream out(csvPath.c_str(), std::ios::app);
    if (!fileExists) {
        out << "FirstName;Name;sex;BirthDay;status;phone;email;information;filename\n";
    }

    out << fields.at("FirstName")    << ";"
        << fields.at("Name")         << ";"
        << fields.at("sex")          << ";"
        << fields.at("BirthDay")     << ";"
        << fields.at("status")       << ";"
        << fields.at("phone")        << ";"
        << fields.at("email")        << ";"
        << fields.at("information")  << ";"
        << uploadedFileName << "\n";

    out.close();
}
