#ifndef DBG_ERROR_HPP
#define DBG_ERROR_HPP

#include <stdexcept>
#include <cstring>

namespace dbg {
    class error : public std::runtime_error {
        public:
        [[noreturn]]
        static void send(const std::string& what) {
            throw error(what);
        }

        [[noreturn]]
        static void send_errno(const std::string& prefix) {
            throw error(prefix + ": " + std::strerror(errno));
        }

        private:
        error(const std::string& what) : std::runtime_error(what) {}
    };
}

#endif