#pragma once
#include <cstdint>
#include <optional>

namespace awmtt::xorg
{
    bool open(std::size_t id);
    std::optional<std::size_t> find_free_display(std::size_t max = 100);
} // namespace awmtt::xorg