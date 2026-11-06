#include "RepositoryPaths.hpp"
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <string>

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

    // RS-5: Check that safe paths are cleaned up and accepted.
    if (!check(paths.safeRelativeFile("notes.txt") == "notes.txt",
    "safeRelativeFile() should accept a file name")) {
    return 1;
    }

    if (!check(paths.safeRelativeFile("docs/notes.txt") == "docs/notes.txt",
    "safeRelativeFile() should accept a nested file name")) {
    return 1;
    }

    if (!check(paths.safeRelativeFile("docs/./notes.txt") == "docs/notes.txt",
    "safeRelativeFile() should normalize dot path parts")) {
    return 1;
    }

    if (!check(paths.safeRelativeFile("docs/../notes.txt") == "notes.txt",
    "safeRelativeFile() should normalize parent path parts within the project")) {
    return 1;
    }

    if (!check(paths.safeRelativeFile("my..notes.txt") == "my..notes.txt",
    "safeRelativeFile() should allow names containing consecutive dots")) {
    return 1;
    }

    // RS-5: Check that unsafe paths throw an error.
    const auto rejectsPath = [&paths](const std::string& argument) {
        try {
            paths.safeRelativeFile(argument);
        } catch (const std::runtime_error&) {
            return true;
        }
        return false;
    };

    // RS-5: Check that empty paths, paths outside the project, and hidden folders are rejected.
    if (!check(rejectsPath(""),
    "safeRelativeFile() should reject an empty path")) {
    return 1;
    }

    if (!check(rejectsPath("."),
    "safeRelativeFile() should reject the current directory")) {
    return 1;
    }

    if (!check(rejectsPath("/notes.txt"),
    "safeRelativeFile() should reject rooted paths")) {
    return 1;
    }

    if (!check(rejectsPath("../notes.txt"),
    "safeRelativeFile() should reject paths outside the project")) {
    return 1;
    }

    if (!check(rejectsPath("docs/../../notes.txt"),
    "safeRelativeFile() should reject normalized paths outside the project")) {
    return 1;
    }

    if (!check(rejectsPath(".minigit/HEAD"),
    "safeRelativeFile() should reject .minigit paths")) {
    return 1;
    }

    if (!check(rejectsPath(".git/config"),
    "safeRelativeFile() should reject .git paths")) {
    return 1;
    }

    if (!check(rejectsPath("docs/.minigit/x"),
    "safeRelativeFile() should reject metadata directories in nested paths")) {
    return 1;
    }

    std::cout << "RepositoryPaths tests passed\n";
    
    return 0;
}