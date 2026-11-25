#include "IndexTable.hpp"
#include <chrono>
#include <filesystem>

#include <iostream>

int main() {
    // IT-1: Create an index table from paths and a file helper.
    const minigit::RepositoryPaths paths("test-project");
    const minigit::FileSystem files;
    const minigit::IndexTable index(paths, files);
    (void)index;

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
