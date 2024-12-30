#include <unistd.h>
#include <fcntl.h>
#include <libdbg/process.hpp>
#include <libdbg/error.hpp>
#include <libdbg/pipe.hpp>
#include <utility>

dbg::pipe::pipe(bool term_on_close)
{
    if (pipe2(fds_, term_on_close ? O_CLOEXEC : 0) < 0)
    {
        dbg::error::send_errno("pipe creation failed");
    }
}

dbg::pipe::~pipe()
{
    close_read();
    close_write();
}

int dbg::pipe::relase_read()
{
    return std::exchange(fds_[read_fd], -1);
}

int dbg::pipe::release_write()
{
    return std::exchange(fds_[write_fd], -1);
}

void dbg::pipe::close_read()
{
    if (fds_[read_fd] != -1)
    {
        close(fds_[read_fd]);
        fds_[read_fd] = -1;
    }
}

void dbg::pipe::close_write()
{
    if (fds_[write_fd] != -1)
    {
        close(fds_[write_fd]);
        fds_[write_fd] = -1;
    }
}

std::vector<std::byte> dbg::pipe::read()
{
    char buf[1024];
    int chars_read;
    if ((chars_read = ::read(fds_[read_fd], buf, sizeof(buf))) < 0)
    {
        error::send_errno("could not read from the pipe");
    }

    auto bytes = reinterpret_cast<std::byte *>(buf);
    return std::vector<std::byte>(bytes, bytes + chars_read);
}

void dbg::pipe::write(std::byte *from, std::size_t bytes)
{
    if (::write(fds_[write_fd], from, bytes) < 0)
    {
        error::send_errno("could not write to the pipe");
    }
}