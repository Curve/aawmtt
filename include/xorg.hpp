#pragma once
#include <future>
#include <memory>
#include <cstdint>
#include <utility>
#include <optional>
#include <stop_token>

namespace awmtt
{
    class display
    {
        struct impl;

      private:
        std::unique_ptr<impl> m_impl;

      private:
        display();

      public:
        display(display &&) noexcept;

      public:
        ~display();

      public:
        void use();

      public:
        static std::optional<std::size_t> find(std::size_t max = 100);
        static std::pair<std::future<display>, std::stop_source> connect(std::size_t id);
    };
} // namespace awmtt