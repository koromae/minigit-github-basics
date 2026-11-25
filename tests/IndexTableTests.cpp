#include "IndexTable.hpp"

#include <iostream>

int main() {
    // IT-1: Create an index table from paths and a file helper.
    const minigit::RepositoryPaths paths("test-project");
    const minigit::FileSystem files;
    const minigit::IndexTable index(paths, files);
    (void)index;

    std::cout << "IndexTable constructor test passed\n";
    return 0;
}
