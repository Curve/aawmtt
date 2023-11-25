#pragma once
#include <memory>
#include <spdlog/spdlog.h>

namespace awmtt
{
    class logger
    {
        struct impl;

      private:
        std::unique_ptr<impl> m_impl;

      private:
        logger();

      public:
        spdlog::logger *operator->();

      public:
        static logger &get();
    };
} // namespace awmtt
