#pragma once

#include <filesystem>
#include <string>

namespace minigit {

class FileSystem {
public:
    // FS-17: Read and return the complete contents of a file.
    std::string readFile(const std::filesystem::path& path) const;
    // FS-18: Copy a file , creating any missing destination folders.
    void copy(const std::filesystem::path& source,
          const std::filesystem::path& target) const;
};

} // namespace minigit
