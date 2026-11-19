// HDR-1: Check that every M2-M5 interface header compiles on its own and
// together. No function is called, so no bodies are needed yet.

#include "CommandLineApp.hpp"
#include "CommitStore.hpp"
#include "FileState.hpp"
#include "IndexTable.hpp"
#include "MiniGitRepository.hpp"
#include "WorkingTree.hpp"

#include <iostream>
#include <type_traits>

int main() {
    // Commit is plain data, so it can be built before CommitStore exists.
    minigit::Commit first{"1", "first commit", ""};
    if (first.id != "1" || !first.parentId.empty()) {
        std::cerr << "FAILED: Commit should hold id, message and parentId\n";
        return 1;
    }

    // MiniGitRepository must not be built from a bare string by accident.
    static_assert(!std::is_convertible<std::filesystem::path,
                                       minigit::MiniGitRepository>::value,
                  "MiniGitRepository constructor should be explicit");

    std::cout << "All interface headers compile\n";
    return 0;
}
