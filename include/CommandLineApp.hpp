#pragma once

#include "MiniGitRepository.hpp"

#include <filesystem>
#include <ostream>
#include <string>
#include <vector>

namespace minigit {

// HDR-1: Reads the command line, calls MiniGitRepository, prints the result
// or "Error: ..." and returns the exit code (0 success, 1 error).
// Passing the streams in lets tests capture the output.
class CommandLineApp {
public:
    // CLA-1 (M2): Build the repository for projectRoot and keep the streams.
    CommandLineApp(const std::filesystem::path& projectRoot,
                   std::ostream& out,
                   std::ostream& err);

    // CLA-1 (M2): Usage for no command or an unknown command, exit code 1.
    // CLA-2 (M2): "init" calls initialize().
    // CLA-3 (M2): Every other command calls requireRepository() first.
    // MR-5 to MR-8 connect add, commit, log and status in their milestones.
    int run(const std::vector<std::string>& args);

private:
    MiniGitRepository repo_;
    std::ostream& out_;
    std::ostream& err_;
};

} // namespace minigit
