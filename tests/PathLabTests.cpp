#include "PathLab.hpp"
#include <iostream>

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

int main() {
    const std::filesystem::path root = "course/repo";

    if (!check(minigit::metadataDirectory(root) == root / ".minigit",
               "metadata directory for ordinary root")) {
        return 1;
    }

    const std::filesystem::path nestedRoot = "course/team/repo";
    if (!check(minigit::metadataDirectory(nestedRoot) == nestedRoot / ".minigit",
               "metadata directory for nested root")) {
        return 1;
    }

    std::cout << "All path tests passed\n";
    return 0;
}
