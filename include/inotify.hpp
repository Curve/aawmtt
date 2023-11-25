#pragma once
#include <string>
#include <memory>
#include <optional>
#include <functional>
#include <filesystem>

namespace awmtt
{
    namespace fs = std::filesystem;

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
        void watch(const fs::path &);

      public:
        void start(std::function<void(const std::string &)> &&, std::chrono::seconds timeout);

      public:
        static std::optional<inotify> init();
    };
} // namespace awmtt
