#include "Manager.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <optional>
#include <print>
#include <sstream>
#include <unordered_set>

auto getTime() -> std::string
{
    auto now = std::chrono::system_clock::now();
    std::time_t current_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_time = *std::localtime(&current_time);
    std::ostringstream oss;
    oss << std::put_time(&local_time, "%d-%b-%y %H:%M:%S");
    return oss.str();
}

namespace
{
inline const std::unordered_set<std::string_view> actions{
    "list",     "add", "update", "delete", "mark-todo", "mark-in-progress",
    "mark-done"};
inline const std::unordered_set<std::string_view> statuses{
    "todo", "in-progress", "done"};

auto parseStatus(std::string_view s) -> std::optional<Status>
{
    std::string lower;
    lower.reserve(s.size());
    for (char c : s)
        lower.push_back(
            static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
    if (lower == "todo")
        return Status::TODO;
    if (lower == "in-progress" || lower == "inprogress")
        return Status::IN_PROGRESS;
    if (lower == "done")
        return Status::DONE;
    return std::nullopt;
}
} // namespace

Todoer::Todoer(std::string path, int argc, char **argv)
    : data_{std::move(path)}, args_{argv + 1, argv + argc}, argc_{argc - 1}
{
    if (argc_ == 0)
    {
        std::println(std::cerr, "Usage:\n"
                                "  ./task-cli add <description> [status]\n"
                                "  ./task-cli update <id> <new description>\n"
                                "  ./task-cli delete <id>\n"
                                "  ./task-cli mark-todo <id>\n"
                                "  ./task-cli mark-in-progress <id>\n"
                                "  ./task-cli mark-done <id>\n"
                                "  ./task-cli list [todo|in-progress|done]");
        std::exit(1);
    }

    if (data_.loadAndValidate())
    {
        load();
        parse();
    }
}

void Todoer::load()
{
    simdjson::dom::parser parser;
    simdjson::dom::array jsonArray =
        parser.parse(data_.getJsonData()).get_array();

    for (simdjson::dom::element element : jsonArray)
    {
        unsigned int id = element["id"].get_uint64().value();
        std::string description{element["description"].get_string().value()};
        std::string raw_status{element["status"].get_string().value()};
        std::string createdAt{element["createdAt"].get_string().value()};
        std::string updatedAt{element["updatedAt"].get_string().value()};

        Status status = Status::TODO;
        if (raw_status == "in-progress")
            status = Status::IN_PROGRESS;
        else if (raw_status == "done")
            status = Status::DONE;

        Task task{id, description, status, createdAt, updatedAt};
        tasks_.emplace(id, std::move(task));
    }
}

void Todoer::parse()
{
    std::string_view action{args_[0]};

    if (action == "add")
        handle_add();
    else if (action == "update")
        handle_update();
    else if (action == "delete")
        handle_delete();
    else if (action.starts_with("mark-"))
        handle_mark();
    else if (action == "list")
        handle_list();
    else
    {
        std::println(std::cerr, "Unknown action: {}", action);
        std::exit(1);
    }
}

auto Todoer::statusToString(Status status) -> std::string_view
{
    switch (status)
    {
    case Status::DONE:
        return "done";
    case Status::TODO:
        return "todo";
    case Status::IN_PROGRESS:
        return "in-progress";
    }
}

void Todoer::handle_list()
{
    std::optional<Status> filter = std::nullopt;
    if (argc_ >= 2)
    {
        filter = parseStatus(args_[1]);
        if (!filter)
        {
            std::println(
                std::cerr,
                "Unknown list filter: {}. Use todo | in-progress | done",
                args_[1]);
            std::exit(1);
        }
    }

    std::println("ID  | Status       | Updated At          | Description");
    std::println("----+--------------+---------------------+-------------------"
                 "----------");

    auto pad = [](std::string_view s, size_t w) -> std::string
    {
        return (s.size() >= w)
                   ? std::string(s.substr(0, w))
                   : std::string(s) + std::string(w - s.size(), ' ');
    };

    size_t shown = 0;
    for (const auto &[id, task] : tasks_)
    {
        if (filter && task.getStatus() != *filter)
            continue;

        std::println("{:<3} | {} | {} | {}", id,
                     pad(statusToString(task.getStatus()), 12),
                     pad(task.getUpdationDate(), 19), task.getDescription());
        ++shown;
    }

    if (shown == 0)
        std::println("(no tasks found)");
}

void Todoer::handle_add()
{
    if (argc_ < 2)
    {
        std::println(std::cerr, "Usage: ./task-cli add <description> [status]");
        std::exit(1);
    }

    std::string description{args_[1]};
    Status initial_status = Status::TODO;

    if (argc_ >= 3)
    {
        auto parsed = parseStatus(args_[2]);
        if (!parsed)
        {
            std::println(std::cerr, "Invalid status parameter: {}", args_[2]);
            std::exit(1);
        }
        initial_status = *parsed;
    }

    unsigned int next_id =
        tasks_.empty() ? 1 : std::prev(tasks_.end())->first + 1;

    std::string current_time = getTime();
    Task task{next_id, std::move(description), initial_status, current_time,
              current_time};

    tasks_.emplace(next_id, std::move(task));
    data_.saveToDisk(tasks_, statusToString);
    std::println("Task added successfully (ID: {})", next_id);
}

void Todoer::handle_update()
{
    if (argc_ < 3)
    {
        std::println(std::cerr,
                     "Usage: ./task-cli update <id> <new description>");
        std::exit(1);
    }

    unsigned int target_id = std::stoul(std::string(args_[1]));
    auto it = tasks_.find(target_id);
    if (it == tasks_.end())
    {
        std::println(std::cerr, "Error: Task ID {} not found.", target_id);
        std::exit(1);
    }

    it->second.setDescription(std::string(args_[2]), getTime());
    data_.saveToDisk(tasks_, statusToString);
    std::println("Task {} updated successfully.", target_id);
}

void Todoer::handle_delete()
{
    if (argc_ < 2)
    {
        std::println(std::cerr, "Usage: ./task-cli delete <id>");
        std::exit(1);
    }

    unsigned int target_id = std::stoul(std::string(args_[1]));
    if (tasks_.erase(target_id) == 0)
    {
        std::println(std::cerr, "Error: Task ID {} does not exist.", target_id);
        std::exit(1);
    }

    data_.saveToDisk(tasks_, statusToString);
    std::println("Task {} deleted successfully.", target_id);
}

void Todoer::handle_mark()
{
    if (argc_ < 2)
    {
        std::println(std::cerr,
                     "Usage: ./task-cli mark-<todo|in-progress|done> <id>");
        std::exit(1);
    }

    std::string_view action{args_[0]};
    unsigned int target_id = std::stoul(std::string(args_[1]));
    auto it = tasks_.find(target_id);

    if (it == tasks_.end())
    {
        std::println(std::cerr, "Error: Task ID {} not found.", target_id);
        std::exit(1);
    }

    Status new_status = Status::TODO;
    if (action == "mark-in-progress")
        new_status = Status::IN_PROGRESS;
    else if (action == "mark-done")
        new_status = Status::DONE;

    it->second.setStatus(new_status, getTime());
    data_.saveToDisk(tasks_, statusToString);
    std::println("Task {} marked as {}.", target_id,
                 statusToString(new_status));
}