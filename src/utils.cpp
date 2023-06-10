#include "utils.hpp"
#include "logger.hpp"

#include <regex>
#include <cstdlib>
#include <filesystem>

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

        if (!fs::exists(rtn))
        {
            logger::get()->warn("Path '{}' doesn't exist", rtn.string());
            return rtn;
        }

        return fs::canonical(rtn);
    }
} // namespace awmtt