#include "Storage.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <print>

namespace fs = std::filesystem;

TodoStorage::TodoStorage(std::string path) : path_{std::move(path)}
{
    ensureFileExists();
}

auto TodoStorage::ensureFileExists() -> void
{
    fs::path file_path(path_);
    if (file_path.has_parent_path() && !fs::exists(file_path.parent_path()))
        fs::create_directories(file_path.parent_path());
    if (!fs::exists(file_path))
    {
        std::ofstream out(file_path, std::ios::out | std::ios::trunc);
        if (out)
            out << "[]\n";
    }
}

auto TodoStorage::loadAndValidate() -> bool
{
    auto error = simdjson::padded_string::load(path_).get(json_);
    if (error)
    {
        std::println(std::cerr, "Storage Error: Could not load file: {}",
                     simdjson::error_message(error));
        return false;
    }

    simdjson::dom::parser parser;
    simdjson::dom::element doc;

    auto parse_error = parser.parse(json_).get(doc);
    if (parse_error)
    {
        std::println(std::cerr, "JSON Structural Error: {}",
                     simdjson::error_message(parse_error));
        return false;
    }

    if (!doc.is_array())
    {
        std::println(std::cerr,
                     "JSON Schema Error: Root element must be a JSON array.");
        return false;
    }
    return true;
}

auto TodoStorage::saveToDisk(const std::flat_map<unsigned int, Task> &tasks,
                             std::string_view (*statusTarget)(Status)) -> bool
{
    std::ofstream out(path_, std::ios::out | std::ios::trunc);
    if (!out)
    {
        std::println(std::cerr,
                     "Storage Error: Could not open file for writing.");
        return false;
    }

    out << "[\n";

    for (auto it = tasks.begin(); it != tasks.end(); ++it)
    {
        const auto &[id, task] = *it;

        out << "  {\n"
            << "    \"id\": " << id << ",\n"
            << R"(    "description": ")" << task.getDescription() << "\",\n"
            << R"(    "status": ")" << statusTarget(task.getStatus()) << "\",\n"
            << R"(    "createdAt": ")" << task.getCreationDate() << "\",\n"
            << R"(    "updatedAt": ")" << task.getUpdationDate() << "\"\n"
            << "  }";

        // Add a comma for all elements except the absolute last one
        if (std::next(it) != tasks.end())
            out << ",";
        out << "\n";
    }

    out << "]\n";
    return true;
}