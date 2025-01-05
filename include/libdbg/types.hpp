#ifndef DBG_TYPES_HPP
#define DBG_TYPES_HPP

#include <array>
#include <cstddef>

namespace dbg
{
    using byte64 = std::array<std::byte, 8>;
    using byte128 = std::array<std::byte, 16>;
} // namespace dbg

#endif