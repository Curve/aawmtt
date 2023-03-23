#include "parser.hpp"
#include "logger.hpp"
#include "constants.hpp"

#include <regex>

namespace awmtt
{
    std::optional<argparse::ArgumentParser> parse(int argc, char **args)
    {
        argparse::ArgumentParser parser(std::string{awmtt::name}, std::string{awmtt::version});

        // Binaries
        parser.add_argument("--xephyr").help("Location of the xephyr binary").default_value("Xephyr").metavar("PATH");
        parser.add_argument("--awesome").help("Location of the awesome binary").default_value("awesome").metavar("PATH");

        // Xephyr Parameters
        parser.add_argument("--display").help("Display to use").metavar("NUMBER").scan<'u', std::size_t>();
        parser.add_argument("--size").help("Xephyr window size").default_value("1920x1080").metavar("SIZE");

        // Awesome Parameters
        parser.add_argument("--config").help("AwesomeWM config to load").default_value("~/.config/awesome/rc.lua").metavar("PATH");

        // Live Reload
        parser.add_argument("--reload").help("Enable auto-reload").default_value(true);
        parser.add_argument("--recursive").help("Recursive Auto-Reload").default_value(true);
        parser.add_argument("--watch").help("Directory to watch for live reload").nargs(0, 1).metavar("PATH");

        try
        {
            parser.parse_args(argc, args);
        }
        catch (const std::runtime_error &err)
        {
            std::cout << parser << std::endl;
            return std::nullopt;
        }

        std::regex validator{R"re(\d+x\d+)re"};

        if (!std::regex_match(parser.get("--size"), validator))
        {
            logger::get()->error("Invalid size: {}", parser.get("--size"));
            std::cout << parser << std::endl;
            return std::nullopt;
        }

        return parser;
    }
} // namespace awmtt