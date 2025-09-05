// TODO: Function for creating date and time (createdAt, updatedAt)
// TODO: Checking if task id already exists and update date and time (updateAt)
// TODO: Function for updating status
// TODO: Function to list all tasks (optionally allow by status)
// TODO: Function to mark status
// TODO: Function to update a task
// TODO: Function to delete a task
// TODO: Fix Makefile

// Future plan: subtasks, tags

#include <iostream>
#include <fstream>
#include <filesystem>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;
using nlohmann::json;

typedef struct Task
{
    int id;
    std::string description;
    std::string status;
    std::string createdAt;
    std::string updatedAt;
} Task;

// Functions
json addTask(json j, Task task);
std::fstream openTasksFile(const fs::path &path);

const std::string filename = "tasks.json";
std::fstream file = openTasksFile(filename);
json jsonData;

int main(int argc, char **argv)
{
    file >> jsonData;
    try
    {
        json jsonData;
        file.seekg(0, std::ios::beg);
        file >> jsonData;

        if (argc < 2)
        {
            std::cerr << "Usage: task-cli add <description> [status]\n";
            return 1;
        }

        const std::string cmd = argv[1];
        if (cmd == "add")
        {
            if (argc < 3)
            {
                std::cerr << "Usage: task-cli add <description> [status]\n";
                return 1;
            }

            const std::string description = argv[2];
            const std::string status = (argc >= 4) ? argv[3] : "NOT_DONE";

            Task t{};
            t.id = 0;
            t.description = description;
            t.status = status;
            t.createdAt = "";
            t.updatedAt = "";

            jsonData = addTask(jsonData, t);

            std::ofstream out(filename, std::ios::out | std::ios::trunc);
            if (!out)
            {
                std::cerr << "Failed to write updated data.\n";
                return 1;
            }
            out << jsonData.dump(4) << '\n';

            std::cout << "Task added.\n";
        }
        else
        {
            std::cerr << "Unknown command: " << cmd << "\n";
            return 1;
        }

        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }
}

std::fstream openTasksFile(const fs::path &path)
{
    if (fs::exists(path))
    {
        std::ifstream in(path, std::ios::in);
        if (!in)
        {
            throw std::runtime_error("Failed to open existing file for validation.");
        }
        const std::string content{std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
        if (!json::accept(content))
        {
            throw std::runtime_error("File exists but is not valid JSON.");
        }
    }
    else
    {
        json defaultSchema = {{"sequence", 0}, {"tasks", json::array()}};
        std::ofstream out(path, std::ios::out | std::ios::trunc);
        if (!out)
        {
            throw std::runtime_error("Failed to create new file.");
        }
        out << defaultSchema.dump(4) << '\n';
    }

    std::fstream file(path, std::ios::in | std::ios::out);
    if (!file)
    {
        throw std::runtime_error("Failed to open file for read/write.");
    }
    return file;
}

json addTask(json j, Task task)
{
    json taskJson = {
        {"id", task.id},
        {"description", task.description},
        {"status", task.status},
        {"createdAt", task.createdAt},
        {"updatedAt", task.updatedAt}};
    j["tasks"].push_back(taskJson);
    return j;
}
