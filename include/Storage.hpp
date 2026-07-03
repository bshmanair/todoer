#pragma once

#include "Task.hpp"
#include <flat_map>
#include <simdjson.h>
#include <string>
#include <string_view>

class TodoStorage
{
  public:
    explicit TodoStorage(std::string path);
    auto loadAndValidate() -> bool;
    auto saveToDisk(const std::flat_map<unsigned int, Task> &tasks,
                    std::string_view (*statusTarget)(Status)) -> bool;
    [[nodiscard]] auto getJsonData() const -> const simdjson::padded_string &
    {
        return json_;
    }

    TodoStorage() = delete;
    TodoStorage(const TodoStorage &) = delete;
    auto operator=(const TodoStorage &) -> TodoStorage & = delete;
    TodoStorage(TodoStorage &&) = delete;
    auto operator=(TodoStorage &&) -> TodoStorage & = delete;
    ~TodoStorage() = default;

  private:
    auto ensureFileExists() -> void;
    std::string path_;
    simdjson::padded_string json_;
};