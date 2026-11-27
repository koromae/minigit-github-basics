#include "MiniGitRepository.hpp"
#include <fstream>
#include <stdexcept>
#include <string>

namespace minigit {

namespace {

// MR-3: Write a new repository file, reporting any failure.
void writeNewFile(const std::filesystem::path& file, const std::string& text) {
    std::ofstream output(file);
    if (!output) {
        throw std::runtime_error("Cannot create repository file: " + file.string());
    }
    output << text;
    output.close();
    if (!output) {
        throw std::runtime_error("Cannot write repository file: " + file.string());
    }
}

// MR-3: Name the first part of the layout that is missing, for the
// "damaged repository" error.
std::string firstMissingPart(const RepositoryPaths& paths) {
    namespace fs = std::filesystem;
    const auto root = paths.metadataRoot();

    if (!fs::is_directory(root)) {
        return ".minigit is not a folder";
    }
    if (!fs::is_directory(root / "stage")) {
        return ".minigit/stage is missing";
    }
    if (!fs::is_directory(root / "commits")) {
        return ".minigit/commits is missing";
    }
    if (!fs::is_regular_file(paths.headFile())) {
        return ".minigit/HEAD is missing";
    }
    if (!fs::is_regular_file(paths.indexFile())) {
        return ".minigit/index.txt is missing";
    }
    return ".minigit/next-id.txt is missing";
}

} // namespace

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
// MR-3: Create a new repository in an empty project. A complete repository
// is left unchanged (returns false). A damaged one, where .minigit exists but
// a part is missing, is an error and is never repaired (HW12).
bool MiniGitRepository::initialize() {
    // An existing complete repository needs no changes.
    if (isInitialized()) {
        return false;
    }

    const auto root = paths_.metadataRoot();

    // .minigit exists but is incomplete: report the first missing part.
    if (std::filesystem::exists(root)) {
        throw std::runtime_error(
            "Damaged MiniGit repository: " + firstMissingPart(paths_) +
            ". Fix or remove .minigit, then run minigit init again.");
    }

    // Create the metadata directory and its subdirectories.
    std::filesystem::create_directories(root / "stage");
    std::filesystem::create_directories(root / "commits");

    // Create the three starting files. The first commit ID will be 1.
    writeNewFile(paths_.headFile(), "");
    writeNewFile(paths_.indexFile(), "");
    writeNewFile(root / "next-id.txt", "1");

    return true;
}

// MR-4: Reject commands when the repository layout is incomplete.
void MiniGitRepository::requireRepository() const {
    if (!isInitialized()) {
        throw std::runtime_error(
            "Not a valid MiniGit repository. Run minigit init first.");
    }
}

// MR-5a: Check the file a user wants to add. Only reads the disk; never
// creates or changes anything, so a failed add leaves .minigit untouched.
std::string MiniGitRepository::checkFileToAdd(const std::string& argument) const {
    // Rejects empty, absolute, outside-the-project and .minigit paths, and
    // returns the cleaned path with / separators (e.g. "./a.txt" -> "a.txt").
    const std::string relativePath = paths_.safeRelativeFile(argument);
    const auto file = paths_.workingFile(relativePath);

    if (!std::filesystem::exists(file)) {
        throw std::runtime_error("File does not exist: " + relativePath);
    }
    if (std::filesystem::is_directory(file)) {
        throw std::runtime_error("Cannot add a folder: " + relativePath);
    }
    if (!std::filesystem::is_regular_file(file)) {
        throw std::runtime_error("Not a regular file: " + relativePath);
    }

    return relativePath;
}

} // namespace minigit
