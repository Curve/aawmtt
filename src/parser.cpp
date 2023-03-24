#include "parser.hpp"

#include "utils.hpp"
#include "logger.hpp"
#include "constants.hpp"

#include <regex>
#include <CLI/CLI.hpp>

namespace awmtt
{
    std::optional<settings> parse(int argc, char **args)
    {
        CLI::App parser{fmt::format("{} - v{}", awmtt::name, awmtt::version)};
        parser.get_formatter()->column_width(60);

        settings rtn{};

        parser.add_option("-x,--xephyr", rtn.xephyr, "Location of xephyr binary")->default_val("Xephyr");
        parser.add_option("-a,--awesome", rtn.awesome, "Location of awesome binary")->default_val("awesome");

        parser.add_option("-d,--display", rtn.display, "The Xorg display to use");
        parser.add_option("-s,--size", rtn.size, "Size for the xephyr window")->default_val("1920x1080")->ignore_case();

        parser.add_option("-r,--reload", rtn.reload, "Enable/Disable auto-reload")->default_val(true);
        parser.add_option("-R,--recursive", rtn.recursive, "Watch files recursively")->default_val(true);

        std::string config;
        parser.add_option("-c,--config", config, "AwesomeWM config to load")->default_val("~/.config/awesome/rc.lua");

        std::optional<std::string> watch;
        parser.add_option("-w,--watch", watch, "Directory to watch for auto-reload");

        parser.add_option("--awesome-args", rtn.awesome_args, "Additional arguments for awesome")->delimiter(',');
        parser.add_option("--xephyr-args", rtn.xephyr_args, "Arguments for xephyr")->delimiter(',')->default_val(std::vector<std::string>{"-ac", "-br", "-noreset"});

        try
        {
            parser.parse(argc, args);
        }
        catch (const CLI::ParseError &e)
        {
            parser.exit(e);
            return std::nullopt;
        }

        std::regex validator{R"re(\d+x\d+)re"};

        if (!std::regex_match(rtn.size, validator))
        {
            logger::get()->error("Invalid size: {}", rtn.size);
            std::cout << parser.help() << std::endl;
            return std::nullopt;
        }

        rtn.config = parse_path(config);

        if (!std::filesystem::exists(rtn.config))
        {
            awmtt::logger::get()->error("Config '{}' doesn't exist", rtn.config.string());
            return std::nullopt;
        }

        if (watch.has_value())
        {
            rtn.watch = parse_path(watch.value());
        }
        else
        {
            rtn.watch = rtn.config.parent_path();
        }

        if (!std::filesystem::exists(rtn.watch))
        {
            awmtt::logger::get()->warn("Watch directory '{}' doesn't exist", rtn.watch.string());
            rtn.reload = false;
        }

        return rtn;
    }
} // namespace awmtt