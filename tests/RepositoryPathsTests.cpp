#include "RepositoryPaths.hpp"
#include <filesystem>
#include <iostream>

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
    }

    //RP-4 (issue8)
bool testCommitDirectory() {
    const std::filesystem::path root = "course/repo";
    const minigit::RepositoryPaths paths(root);
    return check(paths.commitDirectory("3") == root / ".minigit" / "commits" / "3",
                 "commitDirectory(\"3\")");
}

bool testCommittedFile() {
    const std::filesystem::path root = "course/repo";
    const minigit::RepositoryPaths paths(root);
    return check(paths.committedFile("3", "docs/notes.txt") ==
                     root / ".minigit" / "commits" / "3" / "files" / "docs" / "notes.txt",
                 "committedFile(\"3\", \"docs/notes.txt\")");
}

bool testCommittedFileParent() {
    const std::filesystem::path root = "course/repo";
    const minigit::RepositoryPaths paths(root);
    return check(paths.committedFile("3", "a.txt").parent_path() ==
                     paths.commitDirectory("3") / "files",
                 "committedFile(\"3\", \"a.txt\").parent_path()");
}

int main() {
    // Test 1: Ordinary repository root
    const std::filesystem::path root = "course/repo";
    const minigit::RepositoryPaths paths(root);

    if (!check(paths.projectRoot() == root,
    "projectRoot() should return the project root")) {
    return 1;
    }

    if (!check(paths.metadataRoot() == root / ".minigit",
    "metadataRoot() should return the metadata directory")) {
    return 1;
    }

    // Test 2: Nested repository root
    const std::filesystem::path nestedRoot = "course/team/repo";
    const minigit::RepositoryPaths nestedPaths(nestedRoot);
    
    if (!check(nestedPaths.projectRoot() == nestedRoot,
    "projectRoot() should handle nested paths")) {
    return 1;
    }

    if (!check(nestedPaths.metadataRoot() == nestedRoot / ".minigit",
    "metadataRoot() should handle nested paths")) {
    return 1;
    }
    if (!testCommitDirectory()) return 1;
    if (!testCommittedFile()) return 1;
    if (!testCommittedFileParent()) return 1;
    
    std::cout << "RepositoryPaths tests passed\n";
    
    return 0;
}