#include "logger.hpp"
#include "inotify.hpp"

#include <mutex>
#include <vector>
#include <thread>
#include <iostream>

#include <poll.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>

namespace awmtt
{
    struct inotify::impl
    {
        int fd;
        std::vector<int> directories;

      public:
        std::mutex mutex;
        std::function<void(const std::string &)> callback;

      public:
        std::jthread thread;
    };

    inotify::inotify() : m_impl(std::make_unique<impl>()) {}

    inotify::~inotify()
    {
        if (!m_impl)
        {
            return;
        }

        for (const auto &directory : m_impl->directories)
        {
            inotify_rm_watch(m_impl->fd, directory);
        }

        close(m_impl->fd);
    }

    inotify::inotify(inotify &&other) noexcept : m_impl(std::move(other.m_impl))
    {
        other.m_impl.reset();
    }

    void inotify::watch(const std::filesystem::path &path)
    {
        inotify_add_watch(m_impl->fd, path.c_str(), IN_CLOSE_WRITE);
    }

    void inotify::set_callback(std::function<void(const std::string &)> &&callback)
    {
        std::lock_guard lock(m_impl->mutex);
        m_impl->callback = std::move(callback);
    }

    std::optional<inotify> inotify::init(std::chrono::seconds timeout)
    {
        auto fd = inotify_init1(IN_CLOEXEC);

        if (fd < 0)
        {
            return std::nullopt;
        }

        auto rtn = inotify{};
        rtn.m_impl->fd = fd;

        rtn.m_impl->thread = std::jthread{[impl = rtn.m_impl.get(), timeout](const std::stop_token &token) {
            pollfd fs[1] = {pollfd{.fd = impl->fd, .events = POLLIN, .revents = {}}};

            while (!token.stop_requested())
            {
                auto status = poll(fs, 1, static_cast<int>(timeout.count()));

                if (!status)
                {
                    continue;
                }

                if (status == -1)
                {
                    logger::get()->error("poll() failed with: {} ({})", errno, std::strerror(errno)); // NOLINT
                    continue;
                }

                std::size_t len{};
                ioctl(impl->fd, FIONREAD, &len);

                auto buffer = std::make_unique<char[]>(len);
                auto rd = read(impl->fd, buffer.get(), len);

                if (rd == -1)
                {
                    logger::get()->error("read() failed with: {} ({})", errno, std::strerror(errno)); // NOLINT
                    continue;
                }

                if (rd == 0)
                {
                    logger::get()->debug("EOF reached");
                    continue;
                }

                std::lock_guard guard(impl->mutex);
                auto offset = 0u;

                while (offset < len)
                {
                    auto *event = reinterpret_cast<inotify_event *>(buffer.get() + offset);

                    if (impl->callback)
                    {
                        impl->callback(event->name);
                    }

                    offset += sizeof(inotify_event) + event->len;
                }
            }

            logger::get()->debug("inotify finished");
        }};

        return rtn;
    }
} // namespace awmtt