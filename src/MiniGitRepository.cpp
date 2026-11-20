#include "MiniGitRepository.hpp"

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
// MR-2 (isInitialized), 
//MR-3 (initialize) and MR-4 (requireRepository) add
// their bodies here in milestone M2. The other functions follow in M3-M5.

} // namespace minigit
