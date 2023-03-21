#include "xorg.hpp"

#include <X11/Xlib.h>
#include <fmt/format.h>

namespace awmtt
{
    std::optional<int> xorg::find_free_display(std::size_t max)
    {
        //? This is kind of retarded but since I couldn't find any other way to list all displays, we'll have to resort to this...
        for (auto i = 0u; max > i; i++)
        {
            const auto *display = XOpenDisplay(fmt::format(":{}", i).c_str());

            if (display)
            {
                continue;
            }

            return i;
        }

        return std::nullopt;
    }
} // namespace awmtt