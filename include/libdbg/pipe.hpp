#ifndef DBG_PIPE_HPP
#define DBG_PIPE_HPP

#include <vector>
#include <cstddef>

namespace dbg
{
    class pipe
    {
    public:
        explicit pipe(bool term_on_exec);
        ~pipe();

        int get_read() { return fds_[read_fd]; }
        int get_write() { return fds_[write_fd]; }

        int relase_read();
        int release_write();

        void close_read();
        void close_write();

        std::vector<std::byte> read();
        void write(std::byte *from, std::size_t bytes);

    private:
        static constexpr unsigned read_fd = 0;
        static constexpr unsigned write_fd = 1;
        int fds_[2];
    };

} // namespace dbg

#endif