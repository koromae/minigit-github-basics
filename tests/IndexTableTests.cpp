#include "IndexTable.hpp"
#include <chrono>
#include <filesystem>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

class TestDirectory {
public:
    TestDirectory()
        : path_(fs::temp_directory_path() /
                ("minigit-index-read-" +
                 std::to_string(std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()))) {
        fs::create_directories(path_ / ".minigit");
    }

    ~TestDirectory() {
        std::error_code error;
        fs::remove_all(path_, error);
    }

    const fs::path& path() const { return path_; }

private:
    fs::path path_;
};

void writeText(const fs::path& path, const std::string& text) {
    std::ofstream output(path, std::ios::binary | std::ios::trunc);
    output << text;
    if (!output) {
        throw std::runtime_error("Could not write test index file");
    }
}

std::string readText(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

} // namespace

int main() {
    // IT-1: Create an index table from paths and a file helper.
    const TestDirectory directory;
    const minigit::RepositoryPaths paths(directory.path());
    const minigit::FileSystem files;
    const minigit::IndexTable index(paths, files);

    std::cout << "IndexTable constructor test passed\n";
    // IT-4: Check sorting, duplicate removal, and forward slashes.
    const auto id = std::chrono::steady_clock::now().time_since_epoch().count();
    const auto root = std::filesystem::temp_directory_path() /
                  ("minigit-it4-" + std::to_string(id));
    const minigit::RepositoryPaths writePaths(root);
    const minigit::IndexTable writeIndex(writePaths, files);

    writeIndex.write({"z.txt", "folder\\a.txt", "a.txt", "folder/a.txt"});
    const auto contents = files.readFile(writePaths.indexFile());
    std::filesystem::remove_all(root);

    if (contents != "a.txt\nfolder/a.txt\nz.txt\n") {
    std::cerr << "IT-4 failed: index contents differ from expected\n";
    return 1;
}
    return 0;
}
