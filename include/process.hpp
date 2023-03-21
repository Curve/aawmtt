#pragma once
#include <memory>
#include <string>
#include <vector>
#include <filesystem>

namespace awmtt
{
    class process
    {
        struct impl;

      private:
        std::unique_ptr<impl> m_impl;

      public:
        ~process();

      public:
        process(std::vector<std::string> args);

      public:
        void wait();
        void restart();

      public:
        void start(const std::filesystem::path &binary);
    };
} // namespace awmtt