#pragma once
#include <memory>
#include <cstdint>
#include <functional>
#include <filesystem>
#include <tl/expected.hpp>

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
        inotify(inotify &&) noexcept;

      public:
        ~inotify();

      public:
        void start();

      public:
        void watch(const std::filesystem::path &, int flags);
        void on_change(std::function<void(std::uint32_t mask)> &&);

      public:
        static tl::expected<inotify, int> init();
    };
} // namespace awmtt