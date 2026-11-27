#include "IndexTable.hpp"

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

    // IT-2: Read several paths from the index.
    writeText(paths.indexFile(), "first.txt\nfolder/second.txt\nthird.txt\n");
    check(index.read() == std::vector<std::string>{
                              "first.txt", "folder/second.txt", "third.txt"},
          "read() should return all paths in order");

    // IT-2: An empty index should return no paths.
    writeText(paths.indexFile(), "");
    check(index.read().empty(), "read() should return an empty vector");

    // IT-2: Blank lines should not become paths.
    writeText(paths.indexFile(), "first.txt\n\n\r\nsecond.txt\n\n");
    check(index.read() ==
              std::vector<std::string>{"first.txt", "second.txt"},
          "read() should ignore blank lines");

    // IT-2: Remove carriage returns from Windows lines and keep the file intact.
    const std::string windowsLines = "first.txt\r\nsecond.txt\r\n";
    writeText(paths.indexFile(), windowsLines);
    check(index.read() == std::vector<std::string>{"first.txt", "second.txt"},
          "read() should remove trailing carriage returns");
    check(readText(paths.indexFile()) == windowsLines,
          "read() should not change the index file");

    // IT-3: Check whether paths are already tracked.
    const std::vector<std::string> entries{
        "first.txt", "folder/second.txt", "third.txt"};
    const auto originalEntries = entries;
    check(minigit::IndexTable::contains(entries, "folder/second.txt"),
          "contains() should find an existing path");
    check(!minigit::IndexTable::contains(entries, "missing.txt"),
          "contains() should return false for a missing path");
    check(!minigit::IndexTable::contains({}, "first.txt"),
          "contains() should return false for empty entries");
    check(minigit::IndexTable::contains(entries, "third.txt"),
          "contains() should find a path among multiple entries");
    check(entries == originalEntries,
          "contains() should not change the entries");

    if (failures != 0) {
        std::cerr << failures << " IndexTable test(s) failed\n";
        return 1;
    }
    std::cout << "IndexTable tests passed\n";
    return 0;
}
