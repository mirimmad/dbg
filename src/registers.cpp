#include <libdbg/registers.hpp>
#include <libdbg/bit.hpp>
#include <libdbg/process.hpp>
#include <iostream>

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

void dbg::registers::write(const register_info& info, value val) {
    auto bytes = as_bytes(data_);
    std::visit([&](auto& v){
        if(sizeof(v) == info.size) {
            auto val_bytes = as_bytes(v);
            std::copy(val_bytes, val_bytes + sizeof(v), bytes + info.offset);
        } else {
            std::cerr << "dbg::register::write called with"
                        "mismatched register and value sizes" << std::endl;
            std::terminate();
        }
    }, val);
}