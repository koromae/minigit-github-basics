#pragma once

#include <filesystem>
#include <string>

namespace minigit {

class FileSystem {
public:
    // FS-17: Read and return the complete contents of a file.
    std::string readFile(const std::filesystem::path& path) const;
};

} // namespace minigit
