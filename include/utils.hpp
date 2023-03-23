#pragma once
#include <string>
#include <filesystem>

namespace awmtt
{
    std::filesystem::path parse_path(const std::string &path);
} // namespace awmtt