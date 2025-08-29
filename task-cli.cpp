#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

/*
TODO: Check if file exists, if not create it. If file exists but isn't JSON, fail fast.

*/

void fileSafetyCheck()
{
    std::string filename = "tasks.json";
    std::fstream file(filename, std::ios::in | std::ios::out);
    if (std::filesystem::exists(filename) && nlohmann::json::accept(file))
    {
        std::cout << "File exists and is valid.\n";
    }
    else if (!std::filesystem::exists(filename))
    {
        std::cout << "File does not exist, creating with default schema...";
        nlohmann::json defaultSchema = {
            {"tasks", nlohmann::json::array()}};
        std::ofstream newFile(filename);
        newFile << defaultSchema.dump(4);
        newFile.close();

        file.open(filename, std::ios::in | std::ios::out);
    }
    else if (std::filesystem::exists(filename) && !nlohmann::json::accept(file))
    {
        std::cerr << "File exists but is not valid JSON, exiting...\n";
        std::exit(1);
    }
}

int main()
{

    return 0;
}