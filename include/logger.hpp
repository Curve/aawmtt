#pragma once
#include <memory>
#include <spdlog/spdlog.h>

namespace awmtt
{
    class logger
    {
        static std::unique_ptr<logger> m_instance;

      private:
        std::unique_ptr<spdlog::logger> m_logger;

      private:
        logger();

      public:
        spdlog::logger *operator->();

      public:
        static logger &get();
    };
} // namespace awmtt