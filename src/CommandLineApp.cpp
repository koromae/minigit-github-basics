#include "CommandLineApp.hpp"

#include <exception>

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

    try {
        if (command == "init") {
            if (args.size() > 1) {
                err_ << "Error: init takes no arguments\n";
                return 1;
            }

            const bool created = repo_.initialize();
            const std::string metadata = repo_.paths().metadataRoot().string();

            if (created) {
                out_ << "Initialized empty MiniGit repository in "
                     << metadata << "\n";
            } else {
                out_ << "MiniGit repository already exists in "
                     << metadata << "\n";
            }
            return 0;
        }

        // CLA-3: the other commands are added here.

        err_ << "Error: unknown command " << command << "\n";
        printUsage(err_);
        return 1;
    }
    catch (const std::exception& e) {
        err_ << "Error: " << e.what() << "\n";
        return 1;
    }
}

} // namespace minigit