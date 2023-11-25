#include "parser.hpp"

#include "logger.hpp"
#include "constants.hpp"

#include <iostream>

#include <fmt/core.h>
#include <range/v3/all.hpp>

namespace awmtt
{
    void print_usage()
    {
        static constexpr auto usage = R"(
{0} - v{1}

Usage: 
    {0} 
    {0} [...]

Options:
    -h --help                   Print usage
    
    -x --xephyr     <path>      Explicitly specify xephyr path 
    --xephyr-args   <args>      Arguments used to invoke xephyr (Default: "-ac -br -noreset")
    
    -a --awesome    <path>      Explicitly specify awesome path
    --awesome-args  <args>      Additional awesome arguments (e.g. "--screen off")
    
    -c --config     <path>      Location of the awesome config to use (Default: "~/.config/awesome/rc.lua")
    -w --watch      <path>      Directory to watch for changes (Default: Parent directory of config)
    -m --mode       <enum>      Awesome restart strategy ["r" = restart, "s" = SIGHUP] (Default: "s")
    
    -s --size       <string>    Xephyr window size (Default: "1920x1080")
    -d --display    <number>    X11 Display to use (Default: Free Display)
    
    --no-reload                 Disable automatic reload
    --no-recursive              Disable recursive directory watch
)";

        std::cout << fmt::format(usage, constants::name, constants::version) << std::endl;
    };

    template <typename A, typename... T>
    bool has(A &args, T &&...arg)
    {
        return (ranges::contains(args, arg) || ...);
    }

    template <typename A, typename... T>
    auto get(A &args, T &&...arg) -> std::optional<typename A::value_type>
    {
        auto indexed = args | ranges::views::enumerate;

        auto pred = [&](const auto &_a, const auto &_b)
        {
            const auto &[i1, a] = _a;
            const auto &[i2, b] = _b;

            return i1 == i2 - 1 && ((a == arg) || ...);
        };

        auto it = ranges::adjacent_find(indexed, pred);

        if (it == indexed.end())
        {
            return std::nullopt;
        }

        return std::get<1>(*std::next(it));
    }

    fs::path parse_path(std::string path)
    {
        if (auto tilde = path.find('~'); tilde != std::string::npos)
        {
            auto *home = std::getenv("HOME");
            path       = path.replace(tilde, 1, home);
        }

        auto rtn = fs::path{path};

        if (fs::is_symlink(rtn))
        {
            rtn = fs::read_symlink(rtn);
        }

        if (!fs::exists(rtn))
        {
            logger::get()->warn("'{}' does not exist", rtn.string());
            return rtn;
        }

        return fs::canonical(rtn);
    }

    settings parse(int argc, char **argv)
    {
        settings rtn = {
            .xephyr      = "Xephyr",
            .xephyr_args = {"-ac", "-br", "-noreset"},

            .awesome      = "awesome",
            .awesome_args = {},

            .watch   = {},
            .config  = {},
            .restart = strategy::signal,

            .size    = "1920x1080",
            .display = {},

            .reload    = true,
            .recursive = true,
        };

        auto *c_argv = const_cast<const char **>(argv);
        std::vector<std::string> args(c_argv, c_argv + argc);

        if (has(args, "-h", "--help"))
        {
            print_usage();
            exit(0);
        }

        if (auto arg = get(args, "-x", "--xephyr"); arg)
        {
            rtn.xephyr = arg.value();
        }
        if (auto arg = get(args, "--xephyr-args"); arg)
        {
            rtn.xephyr_args = arg.value() | ranges::views::split(' ') | ranges::to<std::vector<std::string>>;
        }

        if (auto arg = get(args, "-a", "--awesome"); arg)
        {
            rtn.awesome = arg.value();
        }
        if (auto arg = get(args, "--awesome-args"); arg)
        {
            rtn.awesome_args = arg.value() | ranges::views::split(' ') | ranges::to<std::vector<std::string>>;
        }

        if (auto arg = get(args, "-c", "--config"); arg)
        {
            rtn.config = parse_path(arg.value());
        }
        else
        {
            rtn.config = parse_path("~/.config/awesome/rc.lua");
        }
        if (auto arg = get(args, "-w", "--watch"); arg)
        {
            rtn.watch = parse_path(arg.value());
        }
        else
        {
            rtn.watch = rtn.config.parent_path();
        }
        if (auto arg = get(args, "-m", "--mode"); arg)
        {
            rtn.restart = arg.value() == "r" ? strategy::restart : strategy::signal;
        }

        if (auto arg = get(args, "-s", "--size"); arg)
        {
            if (!std::regex_match(arg.value(), std::regex{"\\d+x\\d+"}))
            {
                logger::get()->error("Failed to parse size");
                print_usage();
                exit(1);
            }

            rtn.size = arg.value();
        }
        if (auto arg = get(args, "-d", "--display"); arg)
        {
            if (!std::regex_match(arg.value(), std::regex{"\\d+"}))
            {
                logger::get()->error("Failed to parse display");
                print_usage();
                exit(1);
            }

            rtn.display = std::stoll(arg.value());
        }

        if (has(args, "--no-reload"))
        {
            rtn.reload = false;
        }
        if (has(args, "--no-recursive"))
        {
            rtn.recursive = false;
        }

        return rtn;
    }
} // namespace awmtt
