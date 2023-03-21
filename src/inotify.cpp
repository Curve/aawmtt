#include "inotify.hpp"

#include <poll.h>
#include <unistd.h>
#include <sys/inotify.h>

#include <thread>
#include <atomic>
#include <iostream>
#include <functional>
#include <lockpp/lock.hpp>

namespace awmtt
{
    struct inotify::impl
    {
        int fd;
        std::thread thread;
        std::vector<int> watched;

      public:
        std::atomic_bool stop;
        lockpp::lock<std::function<void(std::uint32_t mask)>> callback;
    };

    inotify::inotify() : m_impl(std::make_unique<impl>()) {}

    inotify::inotify(inotify &&other) noexcept : m_impl(std::move(other.m_impl))
    {
        other.m_impl.reset();
    }

    inotify::~inotify()
    {
        if (!m_impl)
        {
            return;
        }

        m_impl->stop = true;

        if (m_impl->thread.joinable())
        {
            m_impl->thread.join();
        }

        for (const auto &watched : m_impl->watched)
        {
            inotify_rm_watch(m_impl->fd, watched);
        }

        close(m_impl->fd);
    }

    void inotify::start()
    {
        constexpr std::uint32_t EVENT_SIZE = sizeof(inotify_event);
        constexpr std::uint32_t EVENT_BUF_LEN = 1024 * (EVENT_SIZE + 16);

        m_impl->thread = std::thread([this]() {
            pollfd fs[1] = {pollfd{.fd = m_impl->fd, .events = POLLIN, .revents = {}}};

            while (!m_impl->stop)
            {
                auto status = poll(fs, 1, 1000);

                if (status == 0)
                {
                    continue;
                }

                if (status == -1)
                {
                    std::cerr << "poll() error: " << errno << std::endl;
                    continue;
                }

                char buffer[EVENT_BUF_LEN];
                auto len = read(m_impl->fd, buffer, EVENT_BUF_LEN);

                if (len < 0)
                {
                    std::cerr << "read() error: " << errno << std::endl;
                    continue;
                }

                for (auto i = 0u; i < len;)
                {
                    auto *event = reinterpret_cast<inotify_event *>(&buffer[i]);
                    auto callback = m_impl->callback.read();

                    if (*callback)
                    {
                        (*callback)(event->mask);
                    }

                    i += EVENT_SIZE + event->len;
                }
            }
        });
    }

    void inotify::watch(const std::filesystem::path &path, int flags)
    {
        m_impl->watched.emplace_back(inotify_add_watch(m_impl->fd, path.c_str(), flags));
    }

    void inotify::on_change(std::function<void(uint32_t mask)> &&callback)
    {
        m_impl->callback.assign(std::move(callback));
    }

    tl::expected<inotify, int> inotify::init()
    {
        // NOLINTNEXTLINE
        auto fd = inotify_init();

        if (fd < 0)
        {
            return tl::make_unexpected(errno);
        }

        auto rtn = inotify{};
        rtn.m_impl->fd = fd;

        return rtn;
    }
} // namespace awmtt