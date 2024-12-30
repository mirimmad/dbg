
#include "catch_amalgamated.hpp"
#include <libdbg/process.hpp>
#include <libdbg/error.hpp>
#include <signal.h>
#include <sys/types.h>

using namespace dbg;

namespace  
{
    bool process_exists(pid_t pid) {
        auto ret = kill(pid, 0);
        return ret != -1 && errno != ESRCH;
    }
}

TEST_CASE("process::launch success", "[process]") {
    // yes is a pre-installed utility on Linux
    auto proc = process::launch("yes");
    REQUIRE(process_exists(proc->pid()));
}

TEST_CASE("process::launch no_such_program", "[process]") {
    REQUIRE_THROWS_AS(process::launch("noprog"), error);
}
