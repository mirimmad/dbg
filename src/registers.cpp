#include <libdbg/bit.hpp>
#include <libdbg/process.hpp>
#include <libdbg/registers.hpp>
#include <iostream>
#include <type_traits>
#include <algorithm>

namespace
{
    template <class T>
    dbg::byte128 widen(const dbg::register_info &info, T t)
    {
        using namespace dbg;
        if constexpr (std::is_floating_point_v<T>)
        {
            if (info.format == register_format::double_float)
            {
                return to_byte128(static_cast<double>(t));
            }
            if (info.format == register_format::long_double)
                return to_byte128(static_cast<long double>(t));
        }
        else if constexpr (std::is_signed_v<T>)
        {
            if (info.format == register_format::uint)
            {
                switch (info.size)
                {
                case 2:
                    return to_byte128(static_cast<std::int16_t>(t));
                case 4:
                    return to_byte128(static_cast<std::int32_t>(t));
                case 8:
                    return to_byte128(static_cast<std::int64_t>(t));
                default:
                    error::send_errno("Invalid size in 'widen'");
                }
            }
        }

        return to_byte128(t);
    }
}

dbg::registers::value dbg::registers::read(const register_info &info) const
{
    auto bytes = as_bytes(data_);

    if (info.format == register_format::uint)
    {
        switch (info.size)
        {
        case 1:
            return from_bytes<std::uint8_t>(bytes + info.offset);
            break;
        case 2:
            return from_bytes<std::uint16_t>(bytes + info.offset);
            break;
        case 4:
            return from_bytes<std::uint32_t>(bytes + info.offset);
            break;
        case 8:
            return from_bytes<std::uint64_t>(bytes + info.offset);
            break;
        default:
            dbg::error::send("Unexpected register size");
            break;
        }
    }
    else if (info.format == register_format::double_float)
    {
        return from_bytes<double>(bytes + info.offset);
    }

    else if (info.format == register_format::long_double)
    {
        return from_bytes<long double>(bytes + info.offset);
    }
    else if (info.format == register_format::vector && info.size == 8)
    {
        return from_bytes<byte64>(bytes + info.offset);
    }
    else
    {
        return from_bytes<byte128>(bytes + info.offset);
    }
}

void dbg::registers::write(const register_info &info, value val)
{
    auto bytes = as_bytes(data_);
    std::visit([&](auto &v)
               {
        if(sizeof(v) <= info.size) {
            auto wide = widen(info, v);
            auto val_bytes = as_bytes(wide);
            std::copy(val_bytes, val_bytes + info.size, bytes + info.offset);
        } else {
            std::cerr << "dbg::register::write called with"
                        "mismatched register and value sizes" << std::endl;
            std::terminate();
        } }, val);

    if (info.type == register_type::fpr)
    {
        proc_->write_fprs(data_.i387);
    }
    else
    {
        // Align the address to 8 byte boundary
        auto aligned_offset = info.offset & ~0b111;
        proc_->write_user_area(aligned_offset, from_bytes<std::uint64_t>(bytes + info.offset));
    }
}
