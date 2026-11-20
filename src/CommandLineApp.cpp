#include "CommandLineApp.hpp"

namespace minigit {

namespace {

void printUsage(std::ostream& err)
{
    err << "Usage: minigit <command>\n"
        << "Commands: init, add, commit, log, status\n";
}

} // namespace

CommandLineApp::CommandLineApp(const std::filesystem::path& projectRoot,
                               std::ostream& out,
                               std::ostream& err)
    : repo_(projectRoot), out_(out), err_(err)
{
}

int CommandLineApp::run(const std::vector<std::string>& args)
{
    if (args.empty()) {
        printUsage(err_);
        return 1;
    }

    const std::string& command = args[0];

    // CLA-2 (init) and CLA-3 (other commands) will be added here.

    err_ << "Error: unknown command " << command << "\n";
    printUsage(err_);
    return 1;
}

} // namespace minigit