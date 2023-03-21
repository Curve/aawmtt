#include "process.hpp"
#include <reproc++/drain.hpp>
#include <reproc++/reproc.hpp>

namespace awmtt
{
    struct process::impl
    {
        std::vector<std::string> args;
        std::filesystem::path last_binary;
        std::unique_ptr<reproc::process> process;
    };

    process::~process()
    {
        if (!m_impl || !m_impl->process)
        {
            return;
        }

        m_impl->process->terminate();
    }

    process::process(std::vector<std::string> args) : m_impl(std::make_unique<impl>())
    {
        m_impl->args = std::move(args);
    }

    void process::wait()
    {
        m_impl->process->wait(reproc::infinite);
    }

    void process::restart()
    {
        m_impl->process->terminate();
        m_impl->process.reset();

        start(m_impl->last_binary);
    }

    void process::start(const std::filesystem::path &binary)
    {
        m_impl->last_binary = binary;

        auto args = m_impl->args;
        args.insert(args.begin(), binary);

        m_impl->process = std::make_unique<reproc::process>();
        m_impl->process->start(args, reproc::options{.redirect = {.parent = true}});
    }
} // namespace awmtt