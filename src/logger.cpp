#include "logger.hpp"
#include "constants.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace awmtt
{
    struct logger::impl
    {
        std::unique_ptr<spdlog::logger> logger;
    };

    logger::logger() : m_impl(std::make_unique<impl>())
    {
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        m_impl->logger = std::make_unique<spdlog::logger>(std::string{constants::name}, sink);
        m_impl->logger->set_level(spdlog::level::trace);
    }

    spdlog::logger *logger::operator->()
    {
        return m_impl->logger.get();
    }

    logger &logger::get()
    {
        static std::unique_ptr<logger> instance;

        if (!instance)
        {
            instance = std::unique_ptr<logger>(new logger);
        }

        return *instance;
    }
} // namespace awmtt
