#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>

namespace awmtt
{
    namespace fs = std::filesystem;

    enum class strategy
    {
        restart,
        signal,
    };

    struct settings
    {
        std::string xephyr;
        std::vector<std::string> xephyr_args;

        std::string awesome;
        std::vector<std::string> awesome_args;

        fs::path watch;
        fs::path config;
        strategy restart;

        std::string size;
        std::optional<std::size_t> display;

        bool reload;
        bool recursive;
    };

    settings parse(int argc, char **argv);
} // namespace awmtt
