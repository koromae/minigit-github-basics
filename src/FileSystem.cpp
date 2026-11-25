#include "FileSystem.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>

namespace minigit {

// FS-17: Read everything in the file, or report if it cannot be read.
std::string FileSystem::readFile(const std::filesystem::path& path) const {
    // On macOS, an ifstream can "open" a directory and read nothing, without
    // any error, so readFile() would return "" instead of reporting a problem.
    // Check for a directory first so every platform reports the error.
    std::error_code error;
    if (std::filesystem::is_directory(path, error)) {
        throw std::runtime_error("Unable to read a directory as a file: " +
                                 path.string());
    }

    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file for reading: " +
                                 path.string());
    }

    std::string contents{std::istreambuf_iterator<char>(file),
                         std::istreambuf_iterator<char>()};
    if (file.bad()) {
        throw std::runtime_error("Unable to read file: " + path.string());
    }

    return contents;
}
// FS-18: copy the complete contents of a file, creating any missing directories.
void FileSystem::copy(const std::filesystem::path& source,
                      const std::filesystem::path& target) const {
    if (!std::filesystem::is_regular_file(source)) {
        throw std::runtime_error("Source file is missing or is not a regular file");
    }

    const auto parent = target.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    std::filesystem::copy_file(source, target);
}
// Copy the file and create any missing destination folders.
void FileSystem::copy(const std::filesystem::path& source,
                      const std::filesystem::path& target) const {
    if (!std::filesystem::is_regular_file(source)) {
        throw std::runtime_error("Source file is missing or is not a regular file");
    }

    const auto parent = target.parent_path();
    if (!parent.empty()) {
        std::filesystem::create_directories(parent);
    }

    // FS-20: Replace the destination when it already exists.
    std::filesystem::copy_file(
        source, target, std::filesystem::copy_options::overwrite_existing);
}


} // namespace minigit
