#pragma once
#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <filesystem>

namespace awmtt
{
    class inotify
    {
        struct impl;

      private:
        std::unique_ptr<impl> m_impl;

      private:
        inotify();

      public:
        ~inotify();

      public:
        inotify(inotify &&) noexcept;

      public:
        void watch(const std::filesystem::path &);

      public:
        void set_callback(std::function<void(const std::string &)> &&);

      public:
        static std::optional<inotify> init(std::chrono::seconds timeout);
    };
} // namespace awmtt