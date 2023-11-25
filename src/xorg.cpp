#include "xorg.hpp"

#include <cstdlib>
#include <X11/Xlib.h>
#include <fmt/format.h>

namespace awmtt
{
    struct display::impl
    {
        Display *display;
    };

    display::display() : m_impl(std::make_unique<impl>()) {}

    display::display(display &&other) noexcept : m_impl(std::exchange(other.m_impl, nullptr)) {}

    display::~display()
    {
        if (!m_impl || !m_impl->display)
        {
            return;
        }

        XCloseDisplay(m_impl->display);
    }

    void display::use()
    {
        const auto *name = XDisplayString(m_impl->display);
        setenv("DISPLAY", name, 1); // NOLINT
    }

    std::optional<std::size_t> display::find(std::size_t max)
    {
        for (auto i = 0u; max > i; i++)
        {
            if (auto *display = XOpenDisplay(fmt::format(":{}", i).c_str()))
            {
                XCloseDisplay(display);
                continue;
            }

            return i;
        }

        return std::nullopt;
    }

    std::pair<std::future<display>, std::stop_source> display::connect(std::size_t id)
    {
        using namespace std::chrono_literals;

        auto name   = fmt::format(":{}", id);
        auto source = std::stop_source{};

        auto fn = [name, token = source.get_token()]()
        {
            Display *disp{};

            while (!token.stop_requested() && !(disp = XOpenDisplay(name.c_str())))
            {
                std::this_thread::sleep_for(100ms);
            }

            display rtn;
            rtn.m_impl->display = disp;

            return rtn;
        };

        return std::make_pair(std::async(std::launch::async, fn), source);
    }
} // namespace awmtt
