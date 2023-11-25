#include "logger.hpp"
#include "inotify.hpp"

#include <vector>
#include <thread>

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

    inotify::inotify(inotify &&other) noexcept : m_impl(std::exchange(other.m_impl, nullptr)) {}

    void inotify::watch(const std::filesystem::path &path)
    {
        inotify_add_watch(m_impl->fd, path.c_str(), IN_CLOSE_WRITE);
    }

    void inotify::start(std::function<void(const std::string &)> &&callback, std::chrono::seconds timeout)
    {
        auto fn = [this, callback, timeout](const std::stop_token &token)
        {
            pollfd fs[] = {
                pollfd{.fd = m_impl->fd, .events = POLLIN, .revents = {}},
            };

            while (!token.stop_requested())
            {
                auto status = poll(fs, 1, static_cast<int>(timeout.count()));

                if (status == 0)
                {
                    continue;
                }

                if (status < 0)
                {
                    logger::get()->error("poll() failed with: {} ({})", errno, std::strerror(errno)); // NOLINT
                    continue;
                }

                std::size_t len{};
                ioctl(m_impl->fd, FIONREAD, &len);

                auto buffer = std::make_unique<char[]>(len);
                auto rd     = read(m_impl->fd, buffer.get(), len);

                if (rd == 0)
                {
                    continue;
                }

                if (rd < 0)
                {
                    logger::get()->warn("read() failed with: {} ({})", errno, std::strerror(errno)); // NOLINT
                    continue;
                }

                for (auto offset = 0u; offset < len;)
                {
                    auto *event = reinterpret_cast<inotify_event *>(buffer.get() + offset);
                    offset += sizeof(inotify_event) + event->len;

                    if (!callback)
                    {
                        continue;
                    }

                    callback(event->name);
                }
            }

            logger::get()->debug("stopped watching");
        };

        m_impl->thread = std::jthread{fn};
    }

    std::optional<inotify> inotify::init()
    {
        auto fd = inotify_init1(IN_CLOEXEC);

        if (fd < 0)
        {
            return std::nullopt;
        }

        auto rtn       = inotify{};
        rtn.m_impl->fd = fd;

        return rtn;
    }
} // namespace awmtt
