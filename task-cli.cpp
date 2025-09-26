#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <vector>
#include <chrono>
#include <iomanip>
#include <sstream>

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
std::fstream openTasksFile(const fs::path &path); // done
std::string getTime();                            // done
// json updateDescription(json j, int id, const std::string newDescription);
// json updateStatus(json j, int id, const std::string newStatus);
const std::string filename = "todo.json";
std::fstream file = openTasksFile(filename);
bool taskExists(json jsonData, int id);

json jsonData;

void debugprint()
{
    std::cout << "This is a debug line and it worked." << std::endl;
}

int main(int argc, char **argv)
{
    try
    {
        json jsonData;
        file.seekg(0, std::ios::beg); // move seek back to scratch (changes after calling openTasksFile(...))
        file >> jsonData;

        if (argc < 2)
        {
            std::cerr << "Usage: ./task-cli add <description> [status]\n";
            return 1;
        }

        const std::string cmd = argv[1];
        if (cmd == "add")
        {
            if (argc < 3)
            {
                std::cerr << "Usage: ./task-cli add <description> [status]\n";
                return 1;
            }

            const std::string description = argv[2];
            const std::string status = (argc >= 4) ? argv[3] : "NOT_DONE";

            Task t{
                t.id = 0,
                t.description = description,
                t.status = status,
                t.createdAt = getTime(),
                t.updatedAt = getTime()};

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
        else if (cmd == "update")
        {
            if (argc < 4)
            {
                std::cerr << "Usage: ./task-cli update <id> [New description]\n";
                return 1;
            }
            int id = std::stoi(argv[2]);
            if (!taskExists(jsonData, id))
            {
                std::cerr << "Task does not exist, aborting...";
                return 1;
            }
            const std::string newDescription = argv[3];
            // updateDescription(jsonData, id, newDescription);
        }
        else if (cmd == "mark-in-progress")
        {
            if (argc != 3)
            {
                std::cerr << "Usage: ./task-cli mark-in-progress <id>";
                return 1;
            }
            int id = std::stoi(argv[2]);
            // updateStatus(jsonData, id, "IN_PROGRESS");
        }
        else if (cmd == "mark-done")
        {
            if (argc != 3)
            {
                std::cerr << "Usage: ./task-cli mark-done <id>";
                return 1;
            }
            int id = std::stoi(argv[2]);
            // updateStatus(jsonData, id, "DONE");
        }
        else if (cmd == "mark-not-done")
        {
            if (argc != 3)
            {
                std::cerr << "Usage: ./task-cli mark-not-done <id>";
                return 1;
            }
            int id = std::stoi(argv[2]);
            // updateStatus(jsonData, id, "NOT_DONE");
        }
        else if (cmd == "list")
        {
            // TBD
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
        json defaultSchema = {{"ids", json::array()}, {"tasks", json::array()}};
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

json addTask(json j, Task task) // needs fix
{
    auto &idsNode = j["ids"];
    if (!idsNode.is_array())
    {
        throw std::runtime_error("\"ids\" must be an array in the JSON schema.");
    }
    std::vector<int> ids = idsNode.get<std::vector<int>>();

    int nextId = 0;
    if (!ids.empty())
    {
        nextId = *std::max_element(ids.begin(), ids.end()) + 1;
    }
    task.id = nextId;

    json taskJson = {
        {"id", task.id},
        {"description", task.description},
        {"status", task.status},
        {"createdAt", task.createdAt},
        {"updatedAt", task.updatedAt}};

    j["tasks"].push_back(taskJson);
    j["ids"].push_back(task.id);

    return j;
}

json getTaskById(const json &j, int id)
{
    if (!j.contains("tasks") || !j["tasks"].is_array())
    {
        throw std::runtime_error("\"tasks\" must be an array in the JSON schema.");
    }

    for (const auto &task : j["tasks"])
    {
        if (task.contains("id") && task["id"].is_number_integer() && task["id"] == id)
        {
            return task;
        }
    }

    throw std::runtime_error("Task with id " + std::to_string(id) + " not found.");
}

/*
json updateDescription(json jsonData, int id, const std::string newDescription)
{
    json task = getTaskById(jsonData, id);
    jsonData["tasks"]
}
*/

bool taskExists(json j, int id)
{
    if (!j.contains("ids") || !j["ids"].is_array())
    {
        return false;
    }
    std::vector<int> ids = j["ids"].get<std::vector<int>>();
    return std::find(ids.begin(), ids.end(), id) != ids.end();
}

std::string getTime()
{
    auto now = std::chrono::system_clock::now();                          // get current time
    std::time_t current_time = std::chrono::system_clock::to_time_t(now); // convert to c-style time (time_t is basically seconds since 1970)
    std::tm local_time = *std::localtime(&current_time);                  // decompose into calendar time (given in struct tm)
    std::ostringstream oss;                                               // make string stream to build formatted string
    oss << std::put_time(&local_time, "%d-%b-%y %H:%M:%S");               // std::put_time writes string into the stream
    std::string time_string = oss.str();                                  // store
    return time_string;
}
