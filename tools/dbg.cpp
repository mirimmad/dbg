#include <libdbg/libdbg.hpp>
#include <libdbg/process.hpp>
#include <libdbg/error.hpp>
#include <vector>
#include <algorithm>
#include <iostream>
#include <readline/readline.h>
#include <readline/history.h>

namespace {
  std::unique_ptr<dbg::process> attach(int argc, const char **argv) {
    pid_t pid = 0;
    if (argc == 3 && argv[1] == std::string_view("-p")) {
        pid = atoi(argv[2]);
        return dbg::process::attach(pid);
    } else {
      const char *program_path = argv[1];
      return dbg::process::launch(program_path);
    }
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

  void print_stop_reason(const dbg::process& process, dbg::stop_reason reason) {
    std::cout << "Process " << process.pid() << ' ';

    switch (reason.reason)
    {
    case dbg::process_state::exited:
      std::cout << "exited with status " << static_cast<int>(reason.info);
      break;
    
    case dbg::process_state::terminated:
      std::cout << "terminated with signal " << sigabbrev_np(reason.info);
      break;
    
    case dbg::process_state::stopped:
      std::cout << "stopped with signal " << sigabbrev_np(reason.info);
      break;
    
    case dbg::process_state::running:
    break;
    }

    std::cout << std::endl;

  }

   void handle_command(std::unique_ptr<dbg::process>& process, std::string_view line) {
      auto args = split(line, ' ');
      auto command = args[0];

      if(is_prefix(command, "continue")) {
        process->resume();
        auto reason = process->wait_on_signal(); 
        print_stop_reason(*process, reason);
      } 
      else {
        std::cerr << "Unknown command\n";
      }
  }


  void main_loop(std::unique_ptr<dbg::process>& process) {
    char * line = nullptr;
    while((line = readline("dbg> ")) != nullptr) {
      std::string line_str;

      if(line == std::string_view("")) {
        free(line);
        if(history_length > 0) {
          line_str = history_list()[history_length - 1]->line;
        }
      } else {
        line_str = line;
        add_history(line);
        free(line);
      }

      if(! line_str.empty()) {
        try {
          handle_command(process, line_str);
        } catch(const dbg::error& err) {
          std::cout << err.what() << '\n';
        }
      }

    }
  }

}

int main(int argc, const  char** argv) {
  if (argc == 1) {
    std::cerr << "No arguments given\n";
    return -1;
  }

  try {
    auto process = attach(argc, argv);
    main_loop(process);
  } catch (const dbg::error& err) {
    std::cout << err.what() << '\n';
  }
} 
