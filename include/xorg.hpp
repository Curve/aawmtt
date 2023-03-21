#pragma once
#include <cstdint>
#include <optional>

namespace awmtt::xorg
{
    std::optional<int> find_free_display(std::size_t max = 100);
} // namespace awmtt::xorg