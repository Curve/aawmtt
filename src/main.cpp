#include "inotify.hpp"
#include "process.hpp"
#include "parser.hpp"
#include "logger.hpp"
#include "utils.hpp"
#include "xorg.hpp"

#include <fmt/format.h>
#include <filesystem>

int main(int argc, char **args)
{
    namespace fs = std::filesystem;
    using namespace std::chrono_literals;

    auto parsed = awmtt::parse(argc, args);

    if (!parsed)
    {
        return 1;
    }

    // TODO: Sig Handler

    auto config = awmtt::parse_path(parsed->get("--config"));
    auto watch = config.parent_path();

    if (auto parsed_watch = parsed->present("--watch"))
    {
        watch = awmtt::parse_path(parsed_watch.value());
    }

    if (!fs::exists(config))
    {
        awmtt::logger::get()->error("Config '{}' doesn't exist", config.string());
        return 1;
    }

    auto reload = parsed->get<bool>("--reload");
    auto recursive = parsed->get<bool>("--recursive");

    if (!fs::exists(watch))
    {
        awmtt::logger::get()->warn("Watch directory '{}' doesn't exist", watch.string());
        reload = false;
    }

    auto display_id = parsed->present<std::size_t>("--display");

    if (!display_id)
    {
        awmtt::logger::get()->info("Searching for free display");
        auto free = awmtt::display::find();

        if (!free)
        {
            awmtt::logger::get()->error("Failed to find free display");
            return 1;
        }

        display_id = free.value();
    }

    awmtt::logger::get()->info("Using display: {}", display_id.value());

    auto size = parsed->get("--size");

    auto xephyr = awmtt::process(parsed->get("--xephyr"));
    xephyr.start({fmt::format(":{}", display_id.value()), "-screen", size, "-ac", "-br", "-noreset"});

    auto [display_fut, stop_source] = awmtt::display::connect(display_id.value());

    if (display_fut.wait_for(5s) != std::future_status::ready)
    {
        awmtt::logger::get()->error("Timed out waiting for display, did xephyr start correctly?");
        stop_source.request_stop();
        return 1;
    }

    awmtt::logger::get()->info("Display is ready!");
    auto display = display_fut.get();
    display.use();

    auto awesome = awmtt::process(parsed->get("--awesome"));
    awesome.start({"-c", config, "--search", config.parent_path()});

    std::optional<awmtt::inotify> inotify;

    if (reload)
    {
        if (auto rtn = awmtt::inotify::init(1s))
        {
            inotify.emplace(std::move(rtn.value()));
            inotify->watch(watch);

            if (recursive)
            {
                for (const auto &entry : fs::recursive_directory_iterator{watch})
                {
                    if (!entry.is_directory())
                    {
                        continue;
                    }

                    inotify->watch(entry);
                }
            }

            inotify->set_callback([&]() {
                awmtt::logger::get()->info("Detected changes, restarting awesome");
                awesome.restart();
            });

            awmtt::logger::get()->debug("Watching for changes");
        }
    }

    awmtt::logger::get()->debug("Waiting for xephyr");
    xephyr.wait();

    return 0;
}