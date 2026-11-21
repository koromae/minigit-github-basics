#include "CommandLineApp.hpp"

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

// CLA-4: Start MiniGit and pass command-line arguments to CommandLineApp.
int main(int argc, char* argv[]) {
    std::vector<std::string> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.emplace_back(argv[i]);
    }

    minigit::CommandLineApp app(std::filesystem::current_path(),
                                std::cout,
                                std::cerr);
    return app.run(arguments);
}
