#include "xorg.hpp"

#include <X11/Xlib.h>
#include <fmt/format.h>

namespace awmtt
{
    bool xorg::open(std::size_t id)
    {
        auto *display = XOpenDisplay(fmt::format(":{}", id).c_str());

        if (!display)
        {
            return false;
        }

        XCloseDisplay(display);
        return true;
    }

    std::optional<std::size_t> xorg::find_free_display(std::size_t max)
    {
        //? This is kind of retarded but since I couldn't find any other way to list all displays, we'll have to resort to this...
        for (auto i = 0u; max > i; i++)
        {
            if (open(i))
            {
                continue;
            }

            return i;
        }

        return std::nullopt;
    }
} // namespace awmtt