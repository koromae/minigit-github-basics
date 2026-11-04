#include "PathLab.hpp"
#include <iostream>
#include <fstream>
#include <iterator>


bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " 
    <<  message << '\n';
            return false;
    }
    return true;
}

int main() {
    // Baseline smoke check: the project and CTest wiring work before students
    // begin the path Issues. Add focused assertions with each Issue.
    std::filesystem::path sample = "course/repo";
    if (sample.filename() != "repo") {
        std::cerr << "path smoke check failed\n";
        return 1;
    }
    const  std::filesystem::path root = "course/repo";
    if (!check(minigit::metadataDirectory(root) == root /".minigit", "metadataDirectory() should append .minigit to the root path")) {
    return 1;
}
const std::filesystem::path nestedRoot = "course/projects/repo";

if (!check(minigit::metadataDirectory(nestedRoot)
           == nestedRoot / ".minigit",
           "metadataDirectory() should handle a nested root path")) {
    return 1;
}

if (!check(minigit::headFilePath(root)
           == root / ".minigit" / "HEAD",
           "headFilePath() should append HEAD inside .minigit")) {
    return 1;
}

if (!check(minigit::headFilePath(nestedRoot)
           == nestedRoot / ".minigit" / "HEAD",
           "headFilePath() should handle a nested root path")) {
    return 1;
}
const auto testRoot = std::filesystem::temp_directory_path()
                    / "minigit-issue3-oury-test";

if (!check(std::filesystem::create_directory(testRoot),
           "Expected a fresh temporary test directory")) {
    return 1;
}
// Test that writeTextFile() creates missing directories and writes the exact text.

const auto filePath = testRoot / "nested" / "deeper" / "message.txt";
const std::string expected = "Hello from MiniGit!\n";
const bool wroteFile = minigit::writeTextFile(filePath, expected);

std::ifstream input(filePath, std::ios::binary);
const bool openedFile = input.is_open();
const std::string actual{std::istreambuf_iterator<char>(input),
                         std::istreambuf_iterator<char>()};
input.close();

std::filesystem::remove_all(testRoot);

if (!check(wroteFile && openedFile && actual == expected,
           "writeTextFile() should create missing folders and write the exact text")) {
    return 1;
}

    std::cout << "PathLab smoke check passed\n";
    return 0;
}
