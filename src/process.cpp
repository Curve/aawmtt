#include "logger.hpp"
#include "process.hpp"

#include <csignal>
#include <optional>
#include <reproc++/reproc.hpp>

namespace awmtt
{
    template <>
    void process::stop<false>();

    template <>
    void process::stop<true>();

    struct process::impl
    {
        std::string binary;
        std::vector<std::string> last_args;
        std::optional<reproc::process> process;
    };

    process::process(std::string binary) : m_impl(std::make_unique<impl>())
    {
        m_impl->binary = std::move(binary);
    }

    process::~process()
    {
        if (!m_impl)
        {
            return;
        }

        stop<true>();
    }

    void process::wait()
    {
        if (!m_impl || !m_impl->process)
        {
            return;
        }

        m_impl->process->wait(reproc::infinite);
    }

    void process::restart()
    {
        if (!m_impl->process)
        {
            return;
        }

        stop<true>();
        start(m_impl->last_args);
    }

    void process::signal(int signal)
    {
        if (!m_impl->process)
        {
            return;
        }

        auto pid = m_impl->process->pid().first;
        kill(pid, signal);
    }

    template <>
    void process::stop<false>()
    {
        if (m_impl->process.has_value())
        {
            return;
        }

        m_impl->process->terminate();
        m_impl->process.reset();
    }

    template <>
    void process::stop<true>()
    {
        if (!m_impl->process)
        {
            return;
        }

        m_impl->process->kill();
        m_impl->process.reset();
    }

    bool process::start(std::vector<std::string> args)
    {
        m_impl->process.emplace();

        auto params = args;
        params.insert(params.begin(), m_impl->binary);

        m_impl->last_args = std::move(args);
        auto status       = m_impl->process->start(params, reproc::options{.redirect{.parent = true}});

        if (!status)
        {
            return true;
        }

        logger::get()->error("Failed to start process '{}': {}", m_impl->binary, status.message());
        return false;
    }
} // namespace awmtt
