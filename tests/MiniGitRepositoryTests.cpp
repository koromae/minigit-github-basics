// Tests for MR-1: the MiniGitRepository constructor.
// MR-2, MR-3 and MR-4 add their tests to this file.

#include "MiniGitRepository.hpp"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>

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

} // namespace

int main() {
    testStoresProjectRoot();
    testBuildsRepositoryLocations();
    testCreatesNothingOnDisk();
    testPathsIsStable();

    if (failures > 0) {
        std::cerr << failures << " MiniGitRepository check(s) failed\n";
        return 1;
    }
    std::cout << "All MiniGitRepository tests passed\n";
    return 0;
}
