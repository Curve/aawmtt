#include "logger.hpp"
#include "constants.hpp"

#include <spdlog/sinks/stdout_color_sinks.h>

namespace awmtt
{
    std::unique_ptr<logger> logger::m_instance;

    logger::logger()
    {
        auto sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

        m_logger = std::make_unique<spdlog::logger>(std::string{awmtt::name}, sink);
        m_logger->set_level(spdlog::level::trace);
    }

    spdlog::logger *logger::operator->()
    {
        return m_logger.get();
    }

    logger &logger::get()
    {
        if (!m_instance)
        {
            m_instance = std::unique_ptr<logger>(new logger);
        }

        return *m_instance;
    }
} // namespace awmtt