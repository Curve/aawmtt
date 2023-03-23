#include "utils.hpp"

#include <filesystem>
#include <regex>
#include <cstdlib>

namespace fs = std::filesystem;

namespace awmtt
{
    fs::path parse_path(const std::string &path)
    {
        static const std::regex tilde{"~"};
        static const auto home = std::getenv("HOME"); // NOLINT

        auto rtn = fs::path{std::regex_replace(path, tilde, home)};

        if (fs::is_symlink(rtn))
        {
            rtn = fs::read_symlink(rtn);
        }

        return fs::canonical(rtn);
    }
} // namespace awmtt