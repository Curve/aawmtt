#pragma once
#include <string>
#include <vector>
#include <memory>

namespace awmtt
{
    class process
    {
        struct impl;
        using args_t = std::vector<std::string>;

      private:
        std::unique_ptr<impl> m_impl;

      public:
        process(std::string binary);

      public:
        ~process();

      public:
        void wait();
        void restart();

      public:
        template <bool Force> void stop();

      public:
        void start(args_t args);
    };
} // namespace awmtt