// Tests for RP-14 (#32) workingFile() and RP-15 (#34) commitFile().
// Each check follows the "Tests to write" list in its issue.

#include "RepositoryPaths.hpp"
#include <chrono>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

namespace fs = std::filesystem;

int failures = 0;

void check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;   // keep going, so cleanup at the end always runs
    }
}

// True if calling 'action' throws std::invalid_argument.
template <typename Action>
bool throwsInvalidArgument(Action action) {
    try {
        action();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

void testWorkingFile() {
    const fs::path root = "course/repo";
    const fs::path nestedRoot = "course/team/repo";
    const minigit::RepositoryPaths paths(root);
    const minigit::RepositoryPaths nestedPaths(nestedRoot);

    // RP-14 test 1: a file in the project root.
    check(paths.workingFile("notes.txt") == root / "notes.txt",
          "workingFile(\"notes.txt\") should be course/repo/notes.txt");

    // RP-14 test 2: a file in a subfolder.
    check(paths.workingFile("docs/plan.txt") == root / "docs" / "plan.txt",
          "workingFile(\"docs/plan.txt\") should be course/repo/docs/plan.txt");

    // RP-14 test 3: a nested project root.
    check(nestedPaths.workingFile("notes.txt") == nestedRoot / "notes.txt",
          "workingFile() should handle a nested project root");

    // RP-14 acceptance 3: the path is in the Working Tree, never inside .minigit.
    check(paths.workingFile("docs/plan.txt").parent_path() != paths.metadataRoot(),
          "workingFile() should not point inside .minigit");

    // RP-14 test 5 (error case): an empty relative path.
    check(throwsInvalidArgument([&] { paths.workingFile(""); }),
          "workingFile(\"\") should throw std::invalid_argument");

    // RP-14 test 6 (error case): an absolute path would replace the root.
    check(throwsInvalidArgument([&] { paths.workingFile("/tmp/notes.txt"); }),
          "workingFile(\"/tmp/notes.txt\") should throw std::invalid_argument");
}

void testCommitFile() {
    const fs::path root = "course/repo";
    const fs::path nestedRoot = "course/team/repo";
    const minigit::RepositoryPaths paths(root);
    const minigit::RepositoryPaths nestedPaths(nestedRoot);

    // RP-15 test 1: a file in the project root.
    check(paths.commitFile("001", "notes.txt") ==
              root / ".minigit" / "commits" / "001" / "files" / "notes.txt",
          "commitFile(\"001\", \"notes.txt\")");

    // RP-15 test 2: a file in a subfolder.
    check(paths.commitFile("001", "docs/plan.txt") ==
              root / ".minigit" / "commits" / "001" / "files" / "docs" / "plan.txt",
          "commitFile(\"001\", \"docs/plan.txt\")");

    // RP-15 test 3: a nested project root with ID 002.
    check(nestedPaths.commitFile("002", "notes.txt") ==
              nestedRoot / ".minigit" / "commits" / "002" / "files" / "notes.txt",
          "commitFile() should handle a nested project root");

    // RP-15 test 4: the same file in two checkpoints gives two different paths.
    check(paths.commitFile("001", "notes.txt") != paths.commitFile("002", "notes.txt"),
          "commitFile() should differ for IDs 001 and 002");

    // RP-15 acceptance 2: built on checkpointDirectory(), inside files/,
    // so a saved file can never collide with message.txt or parent.txt.
    check(paths.commitFile("001", "message.txt").parent_path() ==
              paths.checkpointDirectory("001") / "files",
          "commitFile() should place files under checkpointDirectory(id)/files");

    // RP-15 test 6 (error case): an empty ID.
    check(throwsInvalidArgument([&] { paths.commitFile("", "notes.txt"); }),
          "commitFile(\"\", ...) should throw std::invalid_argument");

    // RP-15 test 7 (error case): an empty relative path.
    check(throwsInvalidArgument([&] { paths.commitFile("001", ""); }),
          "commitFile(\"001\", \"\") should throw std::invalid_argument");

    // RP-15 acceptance 6 (error case): an absolute relative path.
    check(throwsInvalidArgument([&] { paths.commitFile("001", "/tmp/notes.txt"); }),
          "commitFile(\"001\", \"/tmp/notes.txt\") should throw std::invalid_argument");
}

// RP-14 test 4 and RP-15 test 5: the functions only build paths.
// Use a real, empty temporary folder as the project root, call both
// functions, and check that nothing was created.
void testNothingIsCreated() {
    const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
    const fs::path tempRoot =
        fs::temp_directory_path() / ("minigit-rp14-rp15-" + std::to_string(stamp));
    fs::create_directories(tempRoot);

    const minigit::RepositoryPaths paths(tempRoot);

    const fs::path working = paths.workingFile("docs/new.txt");
    check(working == tempRoot / "docs" / "new.txt",
          "workingFile() should return the path of a file that does not exist yet");
    check(!fs::exists(working) && !fs::exists(tempRoot / "docs"),
          "workingFile() should not create the file or its folder");

    const fs::path saved = paths.commitFile("001", "notes.txt");
    check(saved == tempRoot / ".minigit" / "commits" / "001" / "files" / "notes.txt",
          "commitFile() should return the path for a checkpoint that does not exist");
    check(!fs::exists(tempRoot / ".minigit"),
          "commitFile() should not create any checkpoint folder");

    // Always clean up, pass or fail.
    fs::remove_all(tempRoot);
}

int main() {
    testWorkingFile();
    testCommitFile();
    testNothingIsCreated();

    if (failures != 0) {
        std::cerr << failures << " check(s) failed\n";
        return 1;
    }
    std::cout << "workingFile and commitFile tests passed\n";
    return 0;
}
