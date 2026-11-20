// BUILD-1: Placeholder entry point so the minigit target builds in CI.
// CLA-4 replaces this body with:
//     build a CommandLineApp(current folder, std::cout, std::cerr)
//     and return app.run(arguments after the program name).

#include <iostream>

int main() {
    std::cerr << "Error: minigit commands are not implemented yet\n";
    return 1;
}
