#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <filesystem>

namespace awmtt
{
    struct settings
    {
        std::string xephyr;
        std::string awesome;

        std::string size;
        std::optional<std::size_t> display;

        std::filesystem::path config;

        std::vector<std::string> xephyr_args;
        std::vector<std::string> awesome_args;

        bool reload;
        bool recursive;
        std::filesystem::path watch;
    };

    std::optional<settings> parse(int argc, char **args);
} // namespace awmtt