#include "xorg.hpp"
#include "process.hpp"
#include "inotify.hpp"
#include "constants.hpp"

#include <regex>
#include <chrono>
#include <thread>
#include <csignal>
#include <fmt/format.h>
#include <sys/inotify.h>
#include <argparse/argparse.hpp>

namespace fs = std::filesystem;

std::unique_ptr<awmtt::process> xephyr;
std::unique_ptr<awmtt::process> awesome;
std::unique_ptr<awmtt::inotify> watcher;

void setup_sigint()
{
    struct sigaction sig_handler;

    sig_handler.sa_handler = [](int) {
        if (xephyr)
        {
            xephyr.reset();
        }
        if (awesome)
        {
            awesome.reset();
        }
        if (watcher)
        {
            watcher.reset();
        }
    };

    sigemptyset(&sig_handler.sa_mask);
    sig_handler.sa_flags = 0;

    sigaction(SIGINT, &sig_handler, nullptr);
}

int main(int argc, char **argv)
{
    argparse::ArgumentParser parser(std::string{awmtt::name}, std::string{awmtt::version});

    // Binaries
    parser.add_argument("--xephyr").help("Location of the xephyr binary").default_value("Xephyr").metavar("PATH");
    parser.add_argument("--awesome").help("Location of the awesome binary").default_value("awesome").metavar("PATH");

    // Xephyr Parameters
    parser.add_argument("--display").help("Display to use").metavar("NUMBER").scan<'d', std::size_t>();
    parser.add_argument("--size").help("Xephyr window size").default_value("1920x1080").metavar("SIZE");

    // Awesome Parameters
    parser.add_argument("--config").help("AwesomeWM config to load").default_value("~/.config/awesome/rc.lua").metavar("PATH");

    parser.add_argument("--reload").help("Enable auto-reload").implicit_value(true).default_value(true);
    parser.add_argument("--recursive").help("Should auto-reload be recursive").implicit_value(true).default_value(true);
    parser.add_argument("--watch").help("Directory to watch for live reload").default_value("~/.config/awesome").metavar("PATH");

    try
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::runtime_error &err)
    {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        return 1;
    }

    auto size = parser.get("--size");

    if (!std::regex_match(size, std::regex{R"re(\d+x\d+)re"}))
    {
        std::cerr << "Specified size is invalid" << std::endl;
        return 1;
    }

    const auto regex = std::regex{"~"};
    // NOLINTNEXTLINE
    const auto *home = std::getenv("HOME");

    auto reload = parser.get<bool>("--reload");
    auto watch = fs::path{std::regex_replace(parser.get("--watch"), regex, home)};
    auto config = fs::path{std::regex_replace(parser.get("--config"), regex, home)};

    if (reload && fs::is_symlink(watch))
    {
        watch = fs::read_symlink(watch);
        std::cout << "Resolved watch sym-link to " << config << std::endl;
    }

    if (fs::is_symlink((config)))
    {
        config = fs::read_symlink(config);
        std::cout << "Resolved config sym-link to " << config << std::endl;
    }

    auto display = parser.present<int>("--display");

    if (!display)
    {
        if (auto free = awmtt::xorg::find_free_display())
        {
            display = free;
        }
        else
        {
            std::cerr << "Failed to find free display" << std::endl;
            return 1;
        }
    }

    std::cout << "Using display: " << display.value() << std::endl;

    xephyr = std::make_unique<awmtt::process>(std::vector<std::string>{fmt::format(":{}", display.value()), "-screen", size, "-ac", "-br", "-noreset"});
    awesome = std::make_unique<awmtt::process>(std::vector<std::string>{"-c", config.string(), "--search", config.parent_path().string()});

    xephyr->start(parser.get("--xephyr"));

    while (!awmtt::xorg::open(display.value()))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    // NOLINTNEXTLINE
    setenv("DISPLAY", fmt::format(":{}", display.value()).c_str(), true);
    awesome->start(parser.get("--awesome"));

    setup_sigint();

    if (reload)
    {
        auto inotify = awmtt::inotify::init();

        if (!inotify.has_value())
        {
            std::cerr << "Failed to setup inotify watcher: " << inotify.error() << std::endl;
            return 1;
        }

        watcher = std::make_unique<awmtt::inotify>(std::move(inotify.value()));
        watcher->watch(watch, IN_MODIFY);

        for (const auto &entry : fs::recursive_directory_iterator{watch})
        {
            if (!entry.is_directory())
            {
                continue;
            }

            watcher->watch(entry, IN_MODIFY);
        }

        watcher->on_change([](auto) {
            if (!awesome)
            {
                return;
            }

            std::cout << "[aawmtt] Detected change, restarting awesome" << std::endl;
            awesome->restart();
        });

        watcher->start();
    }

    xephyr->wait();

    return 0;
}