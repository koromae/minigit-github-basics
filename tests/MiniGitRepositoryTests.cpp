// Tests for MR-1: the MiniGitRepository constructor.
// MR-2, MR-3 and MR-4 add their tests to this file.

#include "MiniGitRepository.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

namespace {

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;   // keep going, so cleanup at the end always runs
    }
}

// A fresh, empty folder that is removed when the test ends.
class TestDirectory {
public:
    TestDirectory()
        : path_(fs::temp_directory_path() /
                ("minigit-mr1-" +
                 std::to_string(std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()))) {
        fs::create_directories(path_);
    }

    ~TestDirectory() {
        std::error_code error;
        fs::remove_all(path_, error);
    }

    const fs::path& path() const { return path_; }

private:
    fs::path path_;
};

// MR-1 tests 1 and 2: the stored root is the one given, for a plain and a
// nested project root.
void testStoresProjectRoot() {
    const fs::path root = "course/repo";
    const fs::path nestedRoot = "course/team/repo";

    const minigit::MiniGitRepository repo(root);
    const minigit::MiniGitRepository nestedRepo(nestedRoot);

    check(repo.paths().projectRoot() == root,
          "paths().projectRoot() should be course/repo");
    check(nestedRepo.paths().projectRoot() == nestedRoot,
          "paths().projectRoot() should handle a nested root");
}

// MR-1 test 3: the repository locations are built from the root.
void testBuildsRepositoryLocations() {
    const fs::path root = "course/repo";
    const minigit::MiniGitRepository repo(root);

    check(repo.paths().metadataRoot() == root / ".minigit",
          "paths().metadataRoot() should be course/repo/.minigit");
    check(repo.paths().headFile() == root / ".minigit" / "HEAD",
          "paths().headFile() should be course/repo/.minigit/HEAD");
    check(repo.paths().indexFile() == root / ".minigit" / "index.txt",
          "paths().indexFile() should be course/repo/.minigit/index.txt");
}

// MR-1 test 4: constructing creates nothing on disk.
void testCreatesNothingOnDisk() {
    TestDirectory folder;
    const minigit::MiniGitRepository repo(folder.path());

    check(repo.paths().projectRoot() == folder.path(),
          "paths().projectRoot() should be the temporary folder");
    check(!fs::exists(folder.path() / ".minigit"),
          "the constructor should not create .minigit/");
    check(fs::is_empty(folder.path()),
          "the constructor should leave the project folder empty");
}

// MR-1 test 5: paths() returns the same stored object every time.
void testPathsIsStable() {
    const minigit::MiniGitRepository repo("course/repo");
    check(&repo.paths() == &repo.paths(),
          "paths() should return a reference to the stored RepositoryPaths");
}
// MR-2: An empty project is not an initialized repository.
void testEmptyProjectIsNotInitialized() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());

    check(!repo.isInitialized(),
          "An empty project should not be initialized");

    check(fs::is_empty(folder.path()),
          "isInitialized should create nothing");
}

// MR-2: All required directories and files must exist.
void testRequiredRepositoryParts() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    const auto root = repo.paths().metadataRoot();

    // Create the required directories.
    fs::create_directories(root / "stage");
    fs::create_directories(root / "commits");

    // Create the required files and close each stream.
    std::ofstream(root / "HEAD").close();
    std::ofstream(root / "index.txt").close();
    std::ofstream(root / "next-id.txt").close();

    check(repo.isInitialized(),
          "A complete layout should be initialized");

    // Remove each required part separately, then restore it.
    for (const char* name :
         {"stage", "commits", "HEAD", "index.txt", "next-id.txt"}) {
        const auto part = root / name;
        const bool directory = fs::is_directory(part);

        fs::remove(part);

        check(!repo.isInitialized(),
              "A missing required part should make the layout invalid");

        if (directory) {
            fs::create_directory(part);
        } else {
            std::ofstream(part).close();
        }
    }
}
// MR-3: Initialize a fresh repository with the correct contents.
void testInitializeCreatesLayout() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    const auto root = repo.paths().metadataRoot();

    check(repo.initialize(),
          "First initialization should return true");
    check(repo.isInitialized(),
          "Initialization should create the complete layout");

    check(fs::is_empty(root / "stage"),
          "Stage should start empty");
    check(fs::is_empty(root / "commits"),
          "Commits should start empty");
    check(fs::file_size(root / "HEAD") == 0,
          "HEAD should start empty");
    check(fs::file_size(root / "index.txt") == 0,
          "Index should start empty");

    std::ifstream input(root / "next-id.txt");
    std::string id;
    std::getline(input, id);
    check(id == "1", "Next commit ID should start at 1");

    check(!repo.initialize(),
          "Second initialization should return false");
}

// MR-3: Repair a partial layout while preserving existing contents.
void testInitializePreservesExistingFiles() {
    TestDirectory folder;
    minigit::MiniGitRepository repo(folder.path());
    const auto root = repo.paths().metadataRoot();

    fs::create_directories(root);

    // These streams close at the end of each statement.
    std::ofstream(root / "HEAD") << "7";
    std::ofstream(root / "index.txt") << "notes.txt";
    std::ofstream(root / "next-id.txt") << "8";

    check(repo.initialize(),
          "Repairing a partial layout should return true");
    check(repo.isInitialized(),
          "The repaired layout should be complete");

    // Check preservation after another initialization too.
    check(!repo.initialize(),
          "The repaired repository should need no more changes");

    std::string text;

    std::ifstream head(root / "HEAD");
    std::getline(head, text);
    check(text == "7", "Existing HEAD should be preserved");

    std::ifstream index(root / "index.txt");
    std::getline(index, text);
    check(text == "notes.txt", "Existing index should be preserved");

    std::ifstream next(root / "next-id.txt");
    std::getline(next, text);
    check(text == "8", "Existing next ID should be preserved");
}


} // namespace

int main() {
    testStoresProjectRoot();
    testBuildsRepositoryLocations();
    testCreatesNothingOnDisk();
    testPathsIsStable();
    testEmptyProjectIsNotInitialized();
    testRequiredRepositoryParts();
    testInitializeCreatesLayout();
    testInitializePreservesExistingFiles();

    if (failures > 0) {
        std::cerr << failures << " MiniGitRepository check(s) failed\n";
        return 1;
    }
    std::cout << "All MiniGitRepository tests passed\n";
    return 0;
}
