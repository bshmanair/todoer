#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

/*
TODO: Check if file exists, if not create it. If file exists but isn't JSON, fail fast.

*/

typedef struct Task
{
    int id;
    std::string description;
    std::string status;
    std::string createdAt;
    std::string updatedAt;
} Task;

int main()
{

    std::string filename = "tasks.json";
    std::fstream file(filename, std::ios::in | std::ios::out); // open tasks.json

    auto fileExists = std::filesystem::exists(filename);
    auto validJSON = nlohmann::json::accept(file);
    if (fileExists) // confirm valid json
    {
        if (validJSON)
        {
            std::cout << "File exists and is valid.\n";
        }
        else
        {
            std::cerr << "File exists but is not valid JSON, exiting...\n";
            std::exit(1);
        }
    }
    else if (!fileExists) // create default schema if file doesn't exist
    {
        std::cout << "File does not exist, creating with default schema...";
        nlohmann::json defaultSchema = {{"tasks", nlohmann::json::array()}};
        std::ofstream newFile(filename);
        newFile << defaultSchema.dump(4);
        newFile.close();

        file.open(filename, std::ios::in | std::ios::out);
    }

    

    return 0;
}