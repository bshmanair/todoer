#pragma once

#include "Storage.hpp"
#include "Task.hpp"
#include <flat_map>
#include <string>
#include <string_view>
#include <vector>

class Todoer
{
  public:
    Todoer(std::string path, int argc, char **argv);

    Todoer() = delete;
    Todoer(const Todoer &) = delete;
    auto operator=(const Todoer &) -> Todoer & = delete;
    Todoer(Todoer &&) = delete;
    auto operator=(Todoer &&) -> Todoer & = delete;
    ~Todoer() = default;

  private:
    void load();
    void parse();

    void handle_list();
    void handle_add();
    void handle_update();
    void handle_delete();
    void handle_mark();

    static auto statusToString(Status status) -> std::string_view;

    TodoStorage data_;
    std::vector<std::string_view> args_;
    int argc_;
    std::flat_map<unsigned int, Task> tasks_;
};

auto getTime() -> std::string;