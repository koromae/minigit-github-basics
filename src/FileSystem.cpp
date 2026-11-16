#include "FileSystem.hpp"

#include <fstream>
#include <iterator>
#include <stdexcept>

namespace minigit {

// FS-17: Read everything in the file, or report if it cannot be read.
std::string FileSystem::readFile(const std::filesystem::path& path) const {
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

} // namespace minigit
