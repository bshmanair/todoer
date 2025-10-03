/**
 * @file task-cli.cpp
 * @author Bheeshma Nair
 * @brief CLI Task Manager
 * @version 1.0
 * @date 2025-10-03
 */

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
#include <set>
#include <cctype>

namespace fs = std::filesystem;
using nlohmann::json;

struct Task
{
    int id;
    std::string description;
    std::string status; // "todo" | "in-progress" | "done"
    std::string createdAt;
    std::string updatedAt;
};

// All functions declared here
json addTask(json j, Task task);
std::fstream openTasksFile(const fs::path &path);
std::string getTime();
json &updateDescription(json &j, int id, const std::string &newDescription);
bool taskExists(const json &j, int id);
json &updateStatus(json &j, int id, const std::string &newStatus);
void saveJsonToDisk(const json &j, const std::string &outPath);
void listTasks(const json &j, const std::string &filter); // "", "todo", "in-progress", "done"
void printTaskRow(const json &t);
json &deleteTask(json &j, int id);
std::string normalizeStatus(std::string s);

// Global variables
const std::string filename = "todo.json";
json jsonData;

// Main function
int main(int argc, char **argv)
{
    try
    {
        std::fstream file = openTasksFile(filename);

        // Rewind to start and read JSON payload
        file.seekg(0, std::ios::beg);
        if (!(file >> jsonData))
        {
            throw std::runtime_error("Failed to parse JSON file.");
        }

        if (argc < 2)
        {
            std::cerr << "Usage:\n"
                      << "  ./task-cli add <description> [status]\n"
                      << "  ./task-cli update <id> <new description>\n"
                      << "  ./task-cli delete <id>\n"
                      << "  ./task-cli mark-in-progress <id>\n"
                      << "  ./task-cli mark-done <id>\n"
                      << "  ./task-cli mark-not-done <id>\n"
                      << "  ./task-cli list [todo|in-progress|done]\n";
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
            std::string status = (argc >= 4) ? argv[3] : "todo";
            status = normalizeStatus(status);

            if (!(status == "todo" || status == "in-progress" || status == "done"))
            {
                std::cerr << "Invalid status. Use: todo | in-progress | done\n";
                return 1;
            }

            Task t{0, description, status, getTime(), getTime()};
            jsonData = addTask(jsonData, t);
            saveJsonToDisk(jsonData, filename);
            std::cout << "Task added successfully (ID: " << t.id << ")\n";
        }
        else if (cmd == "update")
        {
            if (argc < 4)
            {
                std::cerr << "Usage: ./task-cli update <id> <new description>\n";
                return 1;
            }
            int id = std::stoi(argv[2]);
            if (!taskExists(jsonData, id))
            {
                std::cerr << "Task does not exist, aborting...\n";
                return 1;
            }
            const std::string newDescription = argv[3];
            updateDescription(jsonData, id, newDescription);
            saveJsonToDisk(jsonData, filename);
            std::cout << "Task " << id << " updated.\n";
        }
        else if (cmd == "delete")
        {
            if (argc != 3)
            {
                std::cerr << "Usage: ./task-cli delete <id>\n";
                return 1;
            }
            int id = std::stoi(argv[2]);
            if (!taskExists(jsonData, id))
            {
                std::cerr << "Task does not exist, aborting...\n";
                return 1;
            }
            deleteTask(jsonData, id);
            saveJsonToDisk(jsonData, filename);
            std::cout << "Task " << id << " deleted.\n";
        }
        else if (cmd == "mark-in-progress")
        {
            if (argc != 3)
            {
                std::cerr << "Usage: ./task-cli mark-in-progress <id>\n";
                return 1;
            }
            int id = std::stoi(argv[2]);
            if (!taskExists(jsonData, id))
            {
                std::cerr << "Task does not exist, aborting...\n";
                return 1;
            }
            updateStatus(jsonData, id, "in-progress");
            saveJsonToDisk(jsonData, filename);
            std::cout << "Task " << id << " marked in-progress.\n";
        }
        else if (cmd == "mark-done")
        {
            if (argc != 3)
            {
                std::cerr << "Usage: ./task-cli mark-done <id>\n";
                return 1;
            }
            int id = std::stoi(argv[2]);
            if (!taskExists(jsonData, id))
            {
                std::cerr << "Task does not exist, aborting...\n";
                return 1;
            }
            updateStatus(jsonData, id, "done");
            saveJsonToDisk(jsonData, filename);
            std::cout << "Task " << id << " marked done.\n";
        }
        else if (cmd == "mark-not-done")
        {
            if (argc != 3)
            {
                std::cerr << "Usage: ./task-cli mark-not-done <id>\n";
                return 1;
            }
            int id = std::stoi(argv[2]);
            if (!taskExists(jsonData, id))
            {
                std::cerr << "Task does not exist, aborting...\n";
                return 1;
            }
            updateStatus(jsonData, id, "todo");
            saveJsonToDisk(jsonData, filename);
            std::cout << "Task " << id << " marked todo.\n";
        }
        else if (cmd == "list")
        {
            std::string filter = "";
            if (argc >= 3)
            {
                filter = normalizeStatus(argv[2]);
                const std::set<std::string> allowed = {"todo", "in-progress", "done"};
                if (!filter.empty() && !allowed.count(filter))
                {
                    std::cerr << "Unknown filter. Use: todo | in-progress | done\n";
                    return 1;
                }
            }
            listTasks(jsonData, filter);
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

// Bootstrapping the file into a variable
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
        if (content.empty())
        {
            // Initialize if an empty file somehow exists
            json defaultSchema = {{"ids", json::array()}, {"tasks", json::array()}};
            std::ofstream out(path, std::ios::out | std::ios::trunc);
            if (!out)
                throw std::runtime_error("Failed to re-init empty file.");
            out << defaultSchema.dump(4) << '\n';
        }
        else if (!json::accept(content))
        {
            throw std::runtime_error("File exists but is not valid JSON.");
        }
    }
    else
    {
        json defaultSchema = {{"ids", json::array()}, {"tasks", json::array()}};
        std::ofstream out(path, std::ios::out | std::ios::trunc);
        if (!out)
            throw std::runtime_error("Failed to create new file.");
        out << defaultSchema.dump(4) << '\n';
    }

    std::fstream file(path, std::ios::in | std::ios::out);
    if (!file)
    {
        throw std::runtime_error("Failed to open file for read/write.");
    }
    return file;
}

void saveJsonToDisk(const json &j, const std::string &outPath)
{
    std::ofstream out(outPath, std::ios::out | std::ios::trunc);
    if (!out)
    {
        throw std::runtime_error("Failed to write updated data.");
    }
    out << j.dump(4) << '\n';
}

// Core functions
json addTask(json j, Task task)
{
    // Validate schema
    if (!j.contains("ids") || !j["ids"].is_array())
    {
        j["ids"] = json::array();
    }
    if (!j.contains("tasks") || !j["tasks"].is_array())
    {
        j["tasks"] = json::array();
    }

    std::vector<int> ids = j["ids"].get<std::vector<int>>();
    int nextId = ids.empty() ? 1 : (*std::max_element(ids.begin(), ids.end()) + 1);
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

json &updateDescription(json &j, int id, const std::string &newDescription)
{
    if (!j.contains("tasks") || !j["tasks"].is_array())
    {
        throw std::runtime_error("\"tasks\" must be an array in the JSON schema.");
    }

    for (auto &task : j["tasks"])
    {
        if (task.value("id", -1) == id)
        {
            task["description"] = newDescription;
            task["updatedAt"] = getTime();
            return j;
        }
    }
    throw std::runtime_error("Task id not found");
}

json &updateStatus(json &j, int id, const std::string &newStatus)
{
    const std::set<std::string> allowed = {"todo", "in-progress", "done"};
    if (!allowed.count(newStatus))
    {
        throw std::runtime_error("Invalid status. Use: todo | in-progress | done");
    }

    if (!j.contains("tasks") || !j["tasks"].is_array())
    {
        throw std::runtime_error("\"tasks\" must be an array in the JSON schema.");
    }

    for (auto &task : j["tasks"])
    {
        if (task.value("id", -1) == id)
        {
            task["status"] = newStatus;
            task["updatedAt"] = getTime();
            return j;
        }
    }
    throw std::runtime_error("Task id not found");
}

json &deleteTask(json &j, int id)
{
    if (!j.contains("tasks") || !j["tasks"].is_array())
    {
        throw std::runtime_error("\"tasks\" must be an array in the JSON schema.");
    }
    if (!j.contains("ids") || !j["ids"].is_array())
    {
        j["ids"] = json::array();
    }

    // Erase from tasks
    auto &tasks = j["tasks"];
    tasks.erase(std::remove_if(tasks.begin(), tasks.end(), [&](const json &t)
                               { return t.value("id", -1) == id; }),
                tasks.end());
    // Erase from ids
    auto &ids = j["ids"];
    ids.erase(std::remove_if(ids.begin(), ids.end(), [&](const json &x)
                             { return x.get<int>() == id; }),
              ids.end());
    return j;
}

bool taskExists(const json &j, int id)
{
    if (!j.contains("ids") || !j.at("ids").is_array())
    {
        return false;
    }
    for (const auto &x : j.at("ids"))
    {
        if (x.is_number_integer() && x.get<int>() == id)
        {
            return true;
        }
    }
    return false;
}

// Listing and CLI UI
void listTasks(const json &j, const std::string &filter)
{
    if (!j.contains("tasks") || !j.at("tasks").is_array())
    {
        std::cout << "No tasks.\n";
        return;
    }

    // Header
    std::cout << "ID  | Status       | Updated At          | Description\n";
    std::cout << "----+--------------+---------------------+-----------------------------\n";

    // Collect & sort by id
    std::vector<json> tasks = j.at("tasks").get<std::vector<json>>();
    std::sort(tasks.begin(), tasks.end(),
              [](const json &a, const json &b)
              { return a.value("id", 0) < b.value("id", 0); });

    size_t shown = 0;
    for (const auto &t : tasks)
    {
        const std::string st = t.value("status", "");
        if (!filter.empty() && st != filter)
        {
            continue;
        }
        printTaskRow(t);
        ++shown;
    }

    if (shown == 0)
    {
        if (filter.empty())
        {
            std::cout << "(no tasks)\n";
        }
        else
        {
            std::cout << "(no tasks with status: " << filter << ")\n";
        }
    }
}

void printTaskRow(const json &t)
{
    int id = t.value("id", -1);
    std::string st = t.value("status", "");
    std::string upd = t.value("updatedAt", "");
    std::string desc = t.value("description", "");

    // Very cool pad / truncation
    auto pad = [](const std::string &s, size_t w)
    {
        if (s.size() >= w)
        {
            return s.substr(0, w);
        }
        return s + std::string(w - s.size(), ' ');
    };

    std::cout << std::setw(2) << id << "  | "
              << pad(st, 12) << " | "
              << pad(upd, 19) << " | "
              << desc << "\n";
}

// Helper functions
std::string getTime()
{
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&current_time);
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%d-%b-%y %H:%M:%S");
    return oss.str();
}

std::string normalizeStatus(std::string s)
{
    // Accept common variants
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        char lower = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        out.push_back(lower);
    }

    if (out == "todo" || out == "not_done" || out == "not-done" || out == "notdone")
    {
        return "todo";
    }
    if (out == "in-progress" || out == "inprogress" || out == "in_prog" || out == "in_prog.")
    {
        return "in-progress";
    }
    if (out == "done" || out == "completed" || out == "complete")
    {
        return "done";
    }
    return out; // may be invalid; caller validates
}
