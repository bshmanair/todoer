#include "Manager.hpp"
#include <print>

auto main(int argc, char **argv) -> int
{
    try
    {
        Todoer app("data/tasks.json", argc, argv);
    }
    catch (const std::exception &e)
    {
        std::println(std::cerr, "Fatal Application Error: {}", e.what());
    }
    return 0;
}