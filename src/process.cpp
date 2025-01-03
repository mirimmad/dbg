#include <libdbg/process.hpp>
#include <libdbg/error.hpp>
#include <libdbg/pipe.hpp>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

dbg::process::~process()
{
    if (pid_ != 0)
    {
        int status;
        if (state_ == process_state::running)
        {
            kill(pid_, SIGSTOP);
            waitpid(pid_, &status, 0);
        }

        ptrace(PTRACE_DETACH, pid_, nullptr, nullptr);
        kill(pid_, SIGCONT);

        if (terminate_on_end_)
        {
            kill(pid_, SIGKILL);
            waitpid(pid_, &status, 0);
        }
    }
}

void exit_with_perror(dbg::pipe &channel, std::string const &prefix)
{
    auto message = prefix + ": " + std::strerror(errno);
    channel.write(reinterpret_cast<std::byte *>(message.data()), message.length());
    exit(-1);
}

std::unique_ptr<dbg::process> dbg::process::launch(std::filesystem::path path)
{
    pipe channel(true);
    pid_t pid = 0;
    if ((pid = fork()) < 0)
    {
        dbg::error::send_errno("fork failed");
    }

    if (pid == 0)
    {
        // child closes the read-end (useless)
        channel.close_read();
        // child process
        // enable tracing for the current process
        if (ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0)
        {
            exit_with_perror(channel, "ptrace failed");
        }
        // launch the debugee
        if (execlp(path.c_str(), path.c_str(), nullptr) < 0)
        {
            exit_with_perror(channel, "execlp failed");
        }
    }

    // parent closes the write end
    channel.close_write();
    auto data = channel.read();
    channel.close_read();

    if (data.size() > 0)
    {
        // wait for child process to close
        waitpid(pid, nullptr, 0);
        auto chars = reinterpret_cast<char *>(data.data());
        error::send(std::string(chars, chars + data.size()));
    }

    std::unique_ptr<dbg::process> proc(new dbg::process(pid, true));
    proc->wait_on_signal();
    return proc;
}

std::unique_ptr<dbg::process> dbg::process::attach(pid_t pid)
{
    if (pid == 0)
    {
        error::send("invalid pid");
    }

    if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0)
    {
        error::send_errno("Could not attach.");
    }

    std::unique_ptr<process> proc(new process(pid, false));
    proc->wait_on_signal();
    return proc;
}

void dbg::process::resume()
{
    if (ptrace(PTRACE_CONT, pid_, nullptr, nullptr) < 0)
    {
        error::send_errno("could not resume");
    }
    state_ = process_state::running;
}

dbg::stop_reason dbg::process::wait_on_signal()
{
    int wait_status;
    int options = 0;
    if (waitpid(pid_, &wait_status, options) < 0)
    {
        error::send_errno("waitpid failed");
    }
    stop_reason reason(wait_status);
    state_ = reason.reason;
    return reason;
}

dbg::stop_reason::stop_reason(int wait_status)
{
    if (WIFEXITED(wait_status))
    {
        reason = process_state::exited;
        info = WEXITSTATUS(wait_status);
    }

    else if (WIFSIGNALED(wait_status))
    {
        reason = process_state::terminated;
        info = WTERMSIG(wait_status);
    }

    else if (WIFSTOPPED(wait_status))
    {
        reason = process_state::stopped;
        info = WSTOPSIG(wait_status);
    }
}
