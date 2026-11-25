#include "FileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// FS-17: Check reading regular, empty, multiline, and unavailable files.
namespace {

bool check(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        return false;
    }
    return true;
}

class TestDirectory {
public:
    TestDirectory()
        : path_(std::filesystem::temp_directory_path() /
                ("minigit-fs17-" +
                 std::to_string(std::chrono::steady_clock::now()
                                    .time_since_epoch()
                                    .count()))) {
        std::filesystem::create_directories(path_);
    }

    ~TestDirectory() {
        std::error_code error;
        std::filesystem::remove_all(path_, error);
    }

    const std::filesystem::path& path() const {
        return path_;
    }

private:
    std::filesystem::path path_;
};

bool writeTestFile(const std::filesystem::path& path,
                   const std::string& contents) {
    std::ofstream file(path, std::ios::binary);
    file << contents;
    return file.good();
}

bool reportsReadError(const minigit::FileSystem& fileSystem,
                      const std::filesystem::path& path) {
    try {
        fileSystem.readFile(path);
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

} // namespace

int main() {
    const TestDirectory directory;
    const minigit::FileSystem fileSystem;

    const auto textFile = directory.path() / "text.txt";
    const std::string text = "FS-17 reads file contents.\n";
    if (!check(writeTestFile(textFile, text),
               "test setup should write the text file")) {
        return 1;
    }
    if (!check(fileSystem.readFile(textFile) == text,
               "readFile() should return the complete file contents")) {
        return 1;
    }

    const auto emptyFile = directory.path() / "empty.txt";
    if (!check(writeTestFile(emptyFile, ""),
               "test setup should write the empty file")) {
        return 1;
    }
    if (!check(fileSystem.readFile(emptyFile).empty(),
               "readFile() should return an empty string for an empty file")) {
        return 1;
    }

    const auto multilineFile = directory.path() / "multiline.txt";
    const std::string multiline = "first line\nsecond line\nthird line\n";
    if (!check(writeTestFile(multilineFile, multiline),
               "test setup should write the multiline file")) {
        return 1;
    }
    if (!check(fileSystem.readFile(multilineFile) == multiline,
               "readFile() should preserve all lines")) {
        return 1;
    }

    if (!check(reportsReadError(fileSystem, directory.path() / "missing.txt"),
               "readFile() should report an error for a missing file")) {
        return 1;
    }
    if (!check(reportsReadError(fileSystem, directory.path()),
               "readFile() should report an error for a directory path")) {
        return 1;
    }
    if (!check(reportsReadError(fileSystem, std::filesystem::path{}),
               "readFile() should report an error for an empty path")) {
        return 1;
    }
    // FS-18: Copy a multiline file and preserve the final newline.
    const auto copySource = directory.path() / "copy-source.txt";
    const auto copyTarget = directory.path() / "copy-target.txt";
    const std::string copyContents = "First line\nSecond line\n";

    {
        std::ofstream sourceFile(copySource, std::ios::binary);
        sourceFile << copyContents;
    }

    const minigit::FileSystem copyFileSystem;
    copyFileSystem.copy(copySource, copyTarget);

    if (!check(copyFileSystem.readFile(copyTarget) == copyContents &&
               copyFileSystem.readFile(copySource) == copyContents,
               "copy() should preserve every byte and leave the source unchanged")) {
        return 1;
     }
        // FS-18: Copy an empty file.
    const auto emptyCopySource = directory.path() / "copy-empty-source.txt";
    const auto emptyCopyTarget = directory.path() / "copy-empty-target.txt";

    {
        std::ofstream emptyFile(emptyCopySource, std::ios::binary);
    }

    copyFileSystem.copy(emptyCopySource, emptyCopyTarget);

    if (!check(std::filesystem::exists(emptyCopyTarget) &&
               std::filesystem::file_size(emptyCopyTarget) == 0,
               "copy() should create an empty destination file")) {
        return 1;
    
    }
    // FS-18: Create missing destination folders.
    const auto nestedCopyTarget =
        directory.path() / "new" / "subfolder" / "copy.txt";

    copyFileSystem.copy(copySource, nestedCopyTarget);

    if (!check(std::filesystem::is_directory(nestedCopyTarget.parent_path()) &&
               copyFileSystem.readFile(nestedCopyTarget) == copyContents,
               "copy() should create missing folders and preserve file contents")) {
        return 1;
    }
    // FS-18: Reject a missing source without creating the destination.
    const auto missingCopySource = directory.path() / "missing-copy-source.txt";
    const auto missingCopyTarget = directory.path() / "missing-copy-target.txt";

    bool missingCopyThrew = false;
    try {
        copyFileSystem.copy(missingCopySource, missingCopyTarget);
    } catch (const std::runtime_error&) {
        missingCopyThrew = true;
    }

    if (!check(missingCopyThrew && !std::filesystem::exists(missingCopyTarget),
               "copy() should throw for a missing source and leave no destination file")) {
        return 1;
    }
    // FS-20: Copy again after changing the source.
const std::string updatedContents = "Updated\n";
if (!check(writeTestFile(copySource, updatedContents),
           "test setup should update the source file")) {
    return 1;
}

copyFileSystem.copy(copySource, copyTarget);
if (!check(copyFileSystem.readFile(copyTarget) == updatedContents,
           "copy() should replace the existing destination")) {
    return 1;
}
    std::cout << "FileSystem tests passed\n";
    return 0;
}
