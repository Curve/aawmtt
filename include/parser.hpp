#pragma once
#include <optional>
#include <argparse/argparse.hpp>

namespace awmtt
{
    std::optional<argparse::ArgumentParser> parse(int argc, char **args);
} // namespace awmtt