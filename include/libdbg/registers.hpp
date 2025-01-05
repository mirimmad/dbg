#ifndef DBG_REGISTERS_HPP
#define DBG_REGISTERS_HPP

#include <sys/user.h>
#include <libdbg/register_info.hpp>
#include <libdbg/process.hpp>
#include <libdbg/types.hpp>
#include <variant>

namespace dbg
{
    class registers
    {
    public:
        registers() = delete;
        registers(const registers &r) = delete;
        registers &operator=(const registers &) = delete;

        using value = std::variant<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t,
                                   std::int8_t, std::int16_t, std::int32_t, std::int64_t,
                                   float, double, long double, byte64, byte128>;
        value read(const register_info &info) const;
        void write(const register_info &info, value);

        template <class T>
        T read_by_id_as(register_id id) const {
            return std::get<T>(read(register_info_by(id)));
        }

        void write_by_id(register_id id, value val) {
            write(register_info_by_id(id), val);
        }
        

    private:
        friend process;
        registers(process &proc) : proc_(&proc) {}

        // user from sys/user.h
        user data_;
        process *proc_;
    };
} // namespace dbg

#endif