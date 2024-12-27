#include <libdbg/libdbg.hpp>
#include <iostream>
#include <unistd.h>
#include <string_view>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <vector>
#include <sstream>
#include <algorithm>

namespace {
  pid_t attach(int argc, const char **argv) {
    pid_t pid = 0;
    if (argc == 3 && argv[1] == std::string_view("-p")) {
        pid = atoi(argv[2]);
        if (pid < 0) {
          std::cerr << "Invalid pid\n";
          return -1;
        }
        if (ptrace(PTRACE_ATTACH, pid, nullptr, nullptr) < 0) {
          std::perror("Could not attach");
          return -1;
        }
    } else {
      const char *program_path = argv[1];
      if ((pid = fork()) < 0) {
        std::perror("fork failed");
        return -1;
      }

      if (pid == 0) {
        //child process
        //enable tracing for the current process
        if(ptrace(PTRACE_TRACEME, 0, nullptr, nullptr) < 0) {
          std::perror("Tracing failed");
          return -1;
        }
        //launch the debugee
        if(execlp(program_path, program_path, nullptr) < 0) {
          std::perror("execlp failed");
          return -1;
        }
      }
    }

    return pid;
  }

 
  std::vector<std::string> split(std::string_view str, char delimiter) {
    std::vector<std::string> out{};
    std::stringstream ss {std::string{str}};
    std::string item;

    while(std::getline(ss, item, delimiter)) {
      out.push_back(item);
    }
    return out;
  }

  bool is_prefix(std::string_view command, std::string_view of) {
    if (command.size() > of.size()) return false;
    return std::equal(command.begin(), command.end(), of.begin());
  }

  void resume(pid_t pid) {
    if (ptrace(PTRACE_CONT, pid, nullptr, nullptr) < 0) {
      std::cerr << "Couldn't continie\n";
      std::exit(-1);
    }
  }

  void wait_on_signal(pid_t pid) {
    int wait_status = 0;
    int options = 0;
    if(waitpid(pid, &wait_status, options) < 0) {
      std::perror("waitpid failed");
      std::exit(-1);
    }
  }

   void handle_command(pid_t pid, std::string_view line) {
      auto args = split(line, ' ');
      auto command = args[0];

      if(is_prefix(command, "continue")) {
        resume(pid);
        wait_on_signal(pid); 
      } 
      else {
        std::cerr << "Unknown command\n";
      }
  }

}

int main(int argc, const  char** argv) {
  if (argc == 1) {
    std::cerr << "No arguments given\n";
    return -1;
  }

  pid_t pid = attach(argc, argv);
  int wait_status;
  int options = 0;
  if(waitpid(pid, &wait_status, options) < 0) {
    std::perror("waitpid failed");
    return -1;
  }

  char * line = nullptr;
  while((line = readline("dbg> ")) != nullptr) {
    handle_command(pid, line);
    add_history(line);
    free(line);
  }
} 
