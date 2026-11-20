#include "MiniGitRepository.hpp"
#include <fstream>
#include <stdexcept>

namespace minigit {

// MR-1: Store the project root in RepositoryPaths. Only builds paths;
// nothing is created on disk until initialize() (MR-3) runs.
MiniGitRepository::MiniGitRepository(const std::filesystem::path& projectRoot)
    : paths_(projectRoot) {}

// MR-1: Give callers (CommandLineApp, tests) read-only access to the locations.
const RepositoryPaths& MiniGitRepository::paths() const {
    return paths_;
}
// MR-2: Check that the complete repository layout exists.
// MR-2: check all three directories and three files exist without creating any folder or file. 
bool MiniGitRepository::isInitialized() const {
    const auto root = paths_.metadataRoot();

    return std::filesystem::is_directory(root)
        && std::filesystem::is_directory(root / "stage")
        && std::filesystem::is_directory(root / "commits")
        && std::filesystem::is_regular_file(paths_.headFile())
        && std::filesystem::is_regular_file(paths_.indexFile())
        && std::filesystem::is_regular_file(root / "next-id.txt");
}
// MR-3: Create missing repository parts without overwriting files.
bool MiniGitRepository::initialize() {
    // An existing complete repository needs no changes.
    if (isInitialized()) {
        return false;
    }

    const auto root = paths_.metadataRoot();

    // Create the metadata directory and its subdirectories.
    std::filesystem::create_directories(root / "stage");
    std::filesystem::create_directories(root / "commits");

    for (const char* name : {"HEAD", "index.txt", "next-id.txt"}) {
        const auto file = root / name;

        // Preserve existing files and their contents.
        if (std::filesystem::exists(file)) {
            if (!std::filesystem::is_regular_file(file)) {
                throw std::runtime_error(
                    "Expected a repository file: " + file.string());
            }
            continue;
        }

        // Create only files that are missing.
        std::ofstream output(file);
        if (!output) {
            throw std::runtime_error(
                "Cannot create repository file: " + file.string());
        }

        // The first commit ID will be 1.
        if (file == root / "next-id.txt") {
            output << "1";
        }

        output.close();
        if (!output) {
            throw std::runtime_error(
                "Cannot write repository file: " + file.string());
        }
    }

    return true;
}
// MR-4: Reject commands when the repository layout is incomplete.
void MiniGitRepository::requireRepository() const {
    if (!isInitialized()) {
        throw std::runtime_error(
            "Not a valid MiniGit repository. Run minigit init first.");
    }
}

// MR-2 (isInitialized), 
//MR-3 (initialize) 
//and MR-4 (requireRepository) add
// their bodies here in milestone M2. The other functions follow in M3-M5.

} // namespace minigit
