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
    // RP-3: File at the project root directory check that the path is constructed correctly.
    if (!check(paths.stageFile("notes.txt")
           == root / ".minigit" / "stage" / "notes.txt",
           "stageFile() should handle a top-level file")) {
    return 1;
    }

    // RP-3: File inside a subfolder check that the path is constucted correctly.
    if (!check(paths.stageFile("docs/notes.txt")
           == root / ".minigit" / "stage" / "docs" / "notes.txt",
           "stageFile() should handle a file in a subfolder")) {
    return 1;
    }

    // RP-3: Preserve the filename even when the path has a folder prefix.
    if (!check(paths.stageFile("docs/notes.txt").filename() == "notes.txt",
           "stageFile() should preserve the filename")) {
    return 1;
    }

    std::cout << "RepositoryPaths tests passed\n";
    
    return 0;
}