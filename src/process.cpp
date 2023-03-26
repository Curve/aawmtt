#include "process.hpp"

#include <csignal>
#include <optional>
#include <reproc++/reproc.hpp>

namespace awmtt
{
    template <> void process::stop<false>();
    template <> void process::stop<true>();

    struct process::impl
    {
        std::string binary;
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
        if (!m_impl->process)
        {
            return;
        }

        // NOLINTNEXTLINE
        m_impl->process->wait(reproc::infinite);
    }

    void process::signal(int signal)
    {
        if (!m_impl->process)
        {
            return;
        }

        // NOLINTNEXTLINE
        auto pid = m_impl->process->pid().first;
        kill(pid, signal);
    }

    template <> void process::stop<false>()
    {
        if (!m_impl->process)
        {
            return;
        }

        // NOLINTNEXTLINE
        m_impl->process->terminate();
        m_impl->process.reset();
    }

    template <> void process::stop<true>()
    {
        if (!m_impl->process)
        {
            return;
        }

        // NOLINTNEXTLINE
        m_impl->process->kill();
        m_impl->process.reset();
    }

    void process::start(const args_t &args)
    {
        m_impl->process.emplace();

        auto params = args;
        params.insert(params.begin(), m_impl->binary);

        // NOLINTNEXTLINE
        m_impl->process->start(params, reproc::options{.redirect{.parent = true}});
    }
} // namespace awmtt