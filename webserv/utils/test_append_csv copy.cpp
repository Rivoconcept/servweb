#include <iostream>
#include <map>
#include <fstream>
#include "../include/utils.hpp"

int main()
{
    std::map<std::string, std::string> fields;
    fields["FirstName"] = "Jean";
    // intentionally omit Name and some fields to test missing keys
    fields["sex"] = "M";
    fields["email"] = "jean@example.com";
    std::string csvPath = "./utils/test_contacts.csv";

    // remove file if exists
    std::remove(csvPath.c_str());

    appendToCSV(fields, csvPath, "photo.jpg");
    appendToCSV(fields, csvPath, "photo2.jpg");

    std::ifstream in(csvPath.c_str());
    if (!in.is_open()) {
        std::cerr << "failed to open " << csvPath << std::endl;
        return 1;
    }
    std::string line;
    while (std::getline(in, line)) {
        std::cout << line << std::endl;
    }
    in.close();
    return 0;
}
