#ifndef DBG_PROCESS_HPP
#define DBG_PROCESS_HPP

#include <filesystem>
#include <memory>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include "error.hpp"
#include <libdbg/registers.hpp>

namespace dbg
{
    enum class process_state
    {
        stopped,
        running,
        exited,
        terminated
    };

    struct stop_reason
    {
        stop_reason(int wait_status);
        process_state reason;
        std::uint8_t info;
    };

    class process
    {
    public:
        process() = delete;
        process(const process &) = delete;
        process &operator=(const process &) = delete;

        ~process();

        static std::unique_ptr<process> launch(std::filesystem::path path);

        static std::unique_ptr<process> attach(pid_t pid);

        void resume();
        stop_reason wait_on_signal();

        pid_t pid() const
        {
            return pid_;
        }

        process_state state() const
        {
            return state_;
        }

        registers& get_registers() { return *registers_;}
        const registers& get_registers() const { return *registers_; }

        void write_user_area(std::size_t offset, std::uint64_t data);

    private:
        process(pid_t pid, bool terminate_on_end) : pid_(pid), terminate_on_end_(terminate_on_end), registers_(new registers(*this)) {}
        pid_t pid_ = 0;
        bool terminate_on_end_ = true;
        process_state state_ = process_state::stopped;
        std::unique_ptr<registers> registers_;
        void read_all_registers();
    };
}

#endif