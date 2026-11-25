#include "IndexTable.hpp"

#include <iostream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

} // namespace

int main() {
    // IT-1: Create an index table from paths and a file helper.
    const minigit::RepositoryPaths paths("test-project");
    const minigit::FileSystem files;
    const minigit::IndexTable index(paths, files);
    (void)index;

    // IT-3: Check paths in the index.
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
          "contains() should find a later path");
    check(entries == originalEntries,
          "contains() should not change the entries");

    if (failures != 0) {
        std::cerr << failures << " IndexTable test(s) failed\n";
        return 1;
    }

    std::cout << "IndexTable tests passed\n";
    return 0;
}