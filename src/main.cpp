#include "xorg.hpp"

#include "inotify.hpp"
#include "process.hpp"

#include "parser.hpp"
#include "logger.hpp"
#include "signal.hpp"

#include <fmt/format.h>
#include <fmt/ranges.h>

using namespace awmtt;

int main(int argc, char **argv)
{
    auto settings = parse(argc, argv);

    if (!fs::exists(settings.config))
    {
        logger::get()->error("Config does not exist");
        return 1;
    }

    if (!fs::exists(settings.watch))
    {
        logger::get()->warn("Watch directory does not exist");
        settings.reload = false;
    }

    logger::get()->info("Using restart strategy {}", static_cast<int>(settings.restart));

    if (!settings.display)
    {
        logger::get()->info("Searching for free display");
        auto free = display::find();

        if (!free)
        {
            logger::get()->error("Failed to find free display");
            return 1;
        }

        settings.display = free.value();
    }

    logger::get()->info("Using display: {}", settings.display.value());

    auto xephyr      = process(settings.xephyr);
    auto xephyr_args = std::vector<std::string>{fmt::format(":{}", settings.display.value()), "-screen", settings.size};

    xephyr_args.insert(xephyr_args.end(), settings.xephyr_args.begin(), settings.xephyr_args.end());
    logger::get()->debug("Using xephyr args: {}", fmt::join(xephyr_args, " "));

    if (!xephyr.start(xephyr_args))
    {
        logger::get()->error("Failed to start xephyr");
        return 1;
    }

    auto [display, token] = display::connect(settings.display.value());

    if (auto status = display.wait_for(std::chrono::seconds(5)); status != std::future_status::ready)
    {
        logger::get()->error("Timed out waiting for display, did xephyr start correctly?");
        token.request_stop();
        return 1;
    }

    display.get().use();

    auto awesome      = process(settings.awesome);
    auto awesome_args = std::vector<std::string>{"-c", settings.config, "--search", settings.config.parent_path()};

    awesome_args.insert(awesome_args.end(), settings.awesome_args.begin(), settings.awesome_args.end());
    logger::get()->debug("Using awesome args: {}", fmt::join(awesome_args, " "));

    if (!awesome.start(awesome_args))
    {
        logger::get()->error("Failed to start awesome");
        return 1;
    }

    std::optional<inotify> watcher;

    if (settings.reload)
    {
        auto rtn = inotify::init();

        if (!rtn)
        {
            logger::get()->error("Failed to initialize watcher");
            return 1;
        }

        watcher.emplace(std::move(rtn.value()));
        watcher->watch(settings.watch);

        if (settings.recursive)
        {
            logger::get()->info("Using recursive watch");

            for (const auto &entry : fs::recursive_directory_iterator{settings.watch})
            {
                if (!entry.is_directory())
                {
                    continue;
                }

                watcher->watch(entry);
            }
        }

        watcher->start(
            [&](const auto &file)
            {
                logger::get()->info("Detected changes on '{}', restarting awesome", file);

                if (settings.restart == strategy::signal)
                {
                    awesome.signal(SIGHUP);
                    return;
                }

                awesome.restart();
            },
            std::chrono::seconds(1));

        logger::get()->info("Watching for changes");
    }

    signal::setup(SIGINT,
                  [&]
                  {
                      logger::get()->warn("Received SIGINT, forcefully exiting");

                      awesome.stop<true>();
                      xephyr.stop<true>();

                      watcher.reset();
                  });

    logger::get()->debug("Attached to xephyr");
    xephyr.wait();

    return 0;
}
