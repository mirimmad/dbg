#ifndef DBG_BIT_HPP
#define DBG_BIT_HPP

#include <cstring>
#include <cstddef>
#include <libdbg/types.hpp>

namespace dbg
{
    template <class To>
    To from_bytes(const std::byte* bytes) {
        To ret;
        std::memcpy(&ret, bytes, sizeof(To));
        return ret;
    }

    template <class From>
    std::byte* as_bytes(From& from) {
        return reinterpret_cast<std::byte*>(&from);
    }

    template <class From>
    const std::byte* as_bytes(const From& from) {
        return reinterpret_cast<const std::byte*>(&from);
    }

    template<class From>
    byte128 to_byte128(From from) {
        byte128 ret{};
        std::memcpy(&ret, &from, sizeof(From));
        return ret;
    }

    template<class From>
    byte64 to_bytes64(From from) {
        byte64 ret{};
        std::memcpy(&ret, &from, sizeof(From));
        return ret;
    }


} // namespace dbg


#endif