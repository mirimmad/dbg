
#include "catch_amalgamated.hpp"
#include <libdbg/process.hpp>
#include <libdbg/error.hpp>
#include <signal.h>
#include <sys/types.h>
#include <fstream>

using namespace dbg;

namespace  
{
    bool process_exists(pid_t pid) {
        auto ret = kill(pid, 0);
        return ret != -1 && errno != ESRCH;
    }

    char get_process_status(pid_t pid) {
        std::ifstream stat("/proc" + std::to_string(pid) + "/stat");
        std::string data;
        std::getline(stat, data);

        // Point where the process name ends
        auto index_of_last_paren = data.rfind(')');
        // two poistions past that is the process status
        auto index_of_status_indicator = index_of_last_paren + 2;
        return data[index_of_status_indicator];
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

TEST_CASE("process:launch test_attach", "process") {
    auto proc = process::launch("./hello");
    INFO("status is " << get_process_status(proc->pid()));
    //REQUIRE(get_process_status(proc->pid()) == 't');
}