#pragma once
#include <memory>
#include <functional>

namespace awmtt::signals
{
    template <typename T>
    concept callable = requires(T &t) { t(); };

    template <callable T> void setup(int signal, T &&callback);
} // namespace awmtt::signals

#include "sighandler.inl"