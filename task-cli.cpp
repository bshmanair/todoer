#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

/*
TODO: Check if file exists, if not create it. If file exists but isn't JSON, fail fast.
Not accepting extra arguments beyond the necessiry
*/

enum Status
{
    NOT_DONE,
    DONE,
    IN_PROGRESS
};
typedef struct Task
{
    int id;
    std::string description;
    std::string status;
    std::string createdAt;
    std::string updatedAt;
} Task;

// function for creating date and time
nlohmann::json addTask(nlohmann::json j, std::string task)
{
    j["sequence"] = j["sequence"] + 1;
    Task res;
}

int main(int argc, char **argv)
{

    std::string filename = "tasks.json";
    std::fstream file;

    bool fileExists = std::filesystem::exists(filename);

    if (fileExists) // confirm valid json
    {
        file.open(filename, std::ios::in | std::ios::out);
        if (!file.is_open())
        {
            std::cerr << "Failed to open existing file.\n";
            return 1;
        }

        bool validJSON = nlohmann::json::accept(file);
        if (validJSON)
        {
            std::cout << "File exists and is valid.\n";
        }
        else
        {
            std::cerr << "File exists but is not valid JSON, exiting...\n";
            std::exit(1);
        }
        file.clear();
        file.seekg(0, std::ios::beg);
    }
    else if (!fileExists) // create default schema if file doesn't exist
    {
        std::cout << "File does not exist, creating with default schema...";
        nlohmann::json defaultSchema = {{"sequence", 0}, {"tasks", nlohmann::json::array()}};
        {
            std::ofstream newFile(filename);
            newFile << defaultSchema.dump(4);
        }
        file.open(filename, std::ios::in | std::ios::out);
        if (!file.is_open())
        {
            std::cerr << "Failed to open newly created file.\n";
            return 1;
        }
    }

    // deserialize
    nlohmann::json jsonData = nlohmann::json::parse(file);

    if (argc == 3 && argv[1] == "add")
    {
        jsonData = addTask(jsonData, argv[2]);
    }

    return 0;
}