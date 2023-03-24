#pragma once
#include "sighandler.hpp"

#include <csignal>

namespace awmtt::signals
{
    template <callable T> void setup(int signal, T &&callback)
    {
        static auto g_callback = std::forward<T>(callback);
        struct sigaction action;

        action.sa_handler = [](int) { g_callback(); };

        sigemptyset(&action.sa_mask);
        action.sa_flags = 0;

        sigaction(signal, &action, nullptr);
    }
} // namespace awmtt::signals