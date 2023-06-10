#include "sighandler.hpp"
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

    auto settings = awmtt::parse(argc, args);

    if (!settings)
    {
        return 1;
    }

    awmtt::logger::get()->info("Using restart strategy {}", static_cast<int>(settings->restart_method));

    if (!settings->display)
    {
        awmtt::logger::get()->info("Searching for free display");
        auto free = awmtt::display::find();

        if (!free)
        {
            awmtt::logger::get()->error("Failed to find free display");
            return 1;
        }

        settings->display = free.value();
    }

    awmtt::logger::get()->info("Using display: {}", settings->display.value());

    auto xephyr = awmtt::process(settings->xephyr);

    auto xephyr_args = std::vector<std::string>{fmt::format(":{}", settings->display.value()), "-screen", settings->size};
    xephyr_args.insert(xephyr_args.end(), settings->xephyr_args.begin(), settings->xephyr_args.end());

    awmtt::logger::get()->debug("Using xephyr args: {}", fmt::join(xephyr_args, " "));

    if (!xephyr.start(xephyr_args))
    {
        awmtt::logger::get()->error("Failed to start xephyr");
        return 1;
    }

    auto [display_fut, stop_source] = awmtt::display::connect(settings->display.value());

    if (auto status = display_fut.wait_for(5s); status != std::future_status::ready)
    {
        awmtt::logger::get()->error("Timed out ({}) waiting for display, did xephyr start correctly?", static_cast<int>(status));
        stop_source.request_stop();
        return 1;
    }

    awmtt::logger::get()->info("Display is ready");
    auto display = display_fut.get();
    display.use();

    auto awesome = awmtt::process(settings->awesome);

    auto awesome_args = std::vector<std::string>{"-c", settings->config, "--search", settings->config.parent_path()};
    awesome_args.insert(awesome_args.end(), settings->awesome_args.begin(), settings->awesome_args.end());

    awmtt::logger::get()->debug("Using awesome args: {}", fmt::join(awesome_args, " "));

    if (!awesome.start(awesome_args))
    {
        awmtt::logger::get()->error("Failed to start awesome");
        return 1;
    }

    std::optional<awmtt::inotify> inotify;

    if (settings->reload)
    {
        if (auto rtn = awmtt::inotify::init(1s))
        {
            inotify.emplace(std::move(rtn.value()));
            inotify->watch(settings->watch);

            if (settings->recursive)
            {
                awmtt::logger::get()->info("Using recursive watch");

                for (const auto &entry : fs::recursive_directory_iterator{settings->watch})
                {
                    if (!entry.is_directory())
                    {
                        continue;
                    }

                    inotify->watch(entry);
                }
            }

            inotify->set_callback([&](const std::string &file) {
                awmtt::logger::get()->info("Detected changes on '{}', restarting awesome", file);

                if (settings->restart_method == awmtt::restart_strategy::sighup)
                {
                    awesome.signal(SIGHUP);
                    return;
                }

                awesome.restart();
            });

            awmtt::logger::get()->info("Watching for changes");
        }
    }

    awmtt::signals::setup(SIGINT, [&] {
        awmtt::logger::get()->warn("Received SIGINT, forcefully exiting");
        awesome.stop<true>();
        xephyr.stop<true>();
    });

    awmtt::logger::get()->debug("Attached to xephyr");
    xephyr.wait();

    return 0;
}